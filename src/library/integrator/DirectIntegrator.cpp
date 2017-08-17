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

Spectrum DirectIntegrator::apply(const Ray& in, RenderTile* tile,
								 uint32 pass, ShaderClosure& sc)
{
	return applyRay(in, sc, tile, 0);
}

constexpr float LightEpsilon = 0.00001f;
Spectrum DirectIntegrator::applyRay(const Ray& in, ShaderClosure& sc,
									RenderTile* tile,
									uint32 diffbounces)
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

	// Hemisphere sampling
	for (uint32 i = 0;
		 i < renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
		 ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0.");

		for (uint32 path = 0; path < path_count; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF <= PR_EPSILON || NdotL <= PR_EPSILON)
				continue;

			weight = applyRay(in.next(sc.P, ms.L), other_sc, tile,
							  !std::isinf(ms.PDF) ? diffbounces + 1 : diffbounces);

			weight *= sc.Material->eval(sc, ms.L, NdotL) * NdotL;
			MSI::balance(full_weight, full_pdf, weight, ms.PDF * ms.Weight);
		}
	}

	if (!std::isinf(full_pdf)) {
		// Area sampling!
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd, i);

				const Eigen::Vector3f PS = fps.Point.P - sc.P;
				const Eigen::Vector3f L  = PS.normalized();
				const float NdotL		 = std::abs(L.dot(sc.N)); // No back light detection

				float pdfA = fps.PDF; //MSI::toSolidAngle(fps.PDF, PS.squaredNorm(), NdotL) /*+ sc.Material->pdf(sc, L, NdotL)*/;

				if (pdfA <= PR_EPSILON || NdotL <= PR_EPSILON)
					continue;

				Ray ray = in.next(sc.P, L);
				ray.setFlags(ray.flags() | RF_Light);

				// Full light!!
				if (renderer()->shootWithEmission(weight, ray, other_sc, tile) == light /*&&
					(fps.Point.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
					if(other_sc.Flags & SCF_Inside)// Wrong side (Back side)
						continue;
					
					weight *= sc.Material->eval(sc, L, NdotL) * NdotL;

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
