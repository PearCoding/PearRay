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
	Spectrum applied;
	RenderEntity* entity = renderer()->shootWithEmission(applied, in, sc, tile);

	if (!entity || !sc.Material)
		return applied;

	return applied + applyRay(in, sc, tile, 0);
}

Spectrum DirectIntegrator::applyRay(const Ray& in, const ShaderClosure& sc,
									RenderTile* tile,
									uint32 diffbounces)
{
	if (!sc.Material->canBeShaded())
		return Spectrum();

	float full_pdf = 0;
	Spectrum full_weight;

	// Used temporary
	ShaderClosure other_sc;
	Spectrum weight;

	// Hemisphere sampling
	RandomSampler hemiSampler(tile->random());
	for (uint32 i = 0;
		 i < renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
		 ++i) {
		Spectrum other_weight;
		float other_pdf = 0;

		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0.");

		Eigen::Vector3f rnd = hemiSampler.generate3D(i);
		for (uint32 path = 0; path < path_count; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path);
			const float NdotL   = std::abs(ms.L.dot(sc.N));

			if (ms.PDF <= PR_EPSILON || NdotL <= PR_EPSILON || !(std::isinf(ms.PDF) || diffbounces < renderer()->settings().maxDiffuseBounces()))
				continue;

			Ray ray = in.next(sc.P, ms.L);

			RenderEntity* entity = renderer()->shootWithEmission(weight, ray, other_sc, tile);
			if (entity && other_sc.Material)
				weight += applyRay(ray, other_sc, tile,
								   !std::isinf(ms.PDF) ? diffbounces + 1 : diffbounces);

			weight *= sc.Material->eval(sc, ms.L, NdotL) * NdotL;
			other_weight += ms.Weight * weight;
			other_pdf += ms.PDF;
		}
		MSI::balance(full_weight, full_pdf, other_weight, other_pdf / path_count);
	}

	if (!std::isinf(full_pdf)) {
		Spectrum other_weight;
		// Area sampling!
		for (RenderEntity* light : renderer()->lights()) {
			RandomSampler sampler(tile->random());
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				RenderEntity::FacePointSample fps = light->sampleFacePoint(sampler, i);

				const Eigen::Vector3f PS = fps.Point.P - sc.P;
				const Eigen::Vector3f L  = PS.normalized();
				const float NdotL		 = std::max(0.0f, L.dot(sc.N)); // No back light detection

				float pdf = MSI::toSolidAngle(fps.PDF, PS.squaredNorm(), NdotL) + sc.Material->pdf(sc, L, NdotL);

				if (pdf <= PR_EPSILON)
					continue;

				if (NdotL > PR_EPSILON) {
					Ray ray = in.next(sc.P, L);
					ray.setFlags(ray.flags() | RF_Light);

					if (renderer()->shootWithEmission(other_weight, ray, other_sc, tile) == light) // Full light!!
						other_weight *= sc.Material->eval(sc, L, NdotL) * NdotL;
					else
						other_weight.clear();
				} else {
					other_weight.clear();
				}

				MSI::balance(full_weight, full_pdf, other_weight,
							 std::isinf(pdf) ? 1 : pdf);
			}
		}

		float inf_pdf;
		other_weight = handleInfiniteLights(in, sc, tile, inf_pdf);
		MSI::balance(full_weight, full_pdf, other_weight, inf_pdf);
	}

	return full_weight;
}
}
