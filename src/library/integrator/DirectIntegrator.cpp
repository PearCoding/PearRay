#include "DirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "math/MSI.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "sampler/RandomSampler.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

namespace PR {
DirectIntegrator::DirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

void DirectIntegrator::init() {}

constexpr float LightEpsilon = 0.00001f;
Spectrum DirectIntegrator::apply(const Ray& in,
								 RenderTile* tile,
								 uint32 pass, ShaderClosure& sc)
{
	return applyRay(in, tile, 0, sc);
}

Spectrum DirectIntegrator::applyRay(const Ray& in,
									RenderTile* tile,
									uint32 diffbounces, ShaderClosure& sc)
{
	Spectrum applied;
	RenderEntity* entity = renderer()->shootWithEmission(applied, in, sc, tile);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffbounces > renderer()->settings().maxDiffuseBounces())
		return applied;

	float full_pdf = 0;
	Spectrum full_weight;

	const Eigen::Vector3f rnd = tile->random().get3D();

	// Used temporary
	ShaderClosure other_sc;
	Spectrum weight;

	bool noSpecular = true;

	// Hemisphere sampling
	for (uint32 i = 0;
		 i < renderer()->settings().maxLightSamples() && noSpecular;
		 ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0.");

		for (uint32 path = 0; path < path_count; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF_S <= PR_EPSILON || NdotL <= PR_EPSILON)
				continue;

			weight = applyRay(in.next(sc.P, ms.L), tile,
							  !std::isinf(ms.PDF_S) ? diffbounces + 1 : diffbounces,
							  other_sc);

			//if (!weight.isOnlyZero()) {
			weight *= sc.Material->eval(sc, ms.L, NdotL) * NdotL;
			MSI::balance(full_weight, full_pdf, weight, ms.PDF_S);
			//}

			if(ms.isSpecular())
				noSpecular = false;
		}
	}

	if (noSpecular) {
		// Area sampling!
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd, i);

				const Eigen::Vector3f PS = fps.Point.P - sc.P;
				const Eigen::Vector3f L  = PS.normalized();
				const float NdotL		 = std::abs(L.dot(sc.N)); // No back light detection
				const float lightNdotV   = std::abs(L.dot(fps.Point.Ng));

				float pdfA = fps.PDF_A;

				if (pdfA <= PR_EPSILON || NdotL <= PR_EPSILON || lightNdotV <= PR_EPSILON)
					continue;

				Ray ray = in.next(sc.P, L);
				ray.setFlags(ray.flags() | RF_Light);

				// Full light!!
				if (renderer()->shootWithEmission(weight, ray, other_sc, tile) == light /*&&
					(fps.Point.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
					if (other_sc.Flags & SCF_Inside) // Wrong side (Back side)
						continue;

					weight *= sc.Material->eval(sc, L, NdotL) * NdotL * lightNdotV;

					const float pdfS = MSI::toSolidAngle(pdfA, PS.squaredNorm(), std::abs(sc.NdotV * NdotL));
					MSI::balance(full_weight, full_pdf, weight, pdfS);
				}
			}
		}

		float inf_pdf;
		weight = handleInfiniteLights(in, sc, tile, inf_pdf);
		MSI::balance(full_weight, full_pdf, weight, inf_pdf);
	}

	return applied + full_weight;
}
}
