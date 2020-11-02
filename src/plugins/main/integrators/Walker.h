#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"

#include "IntegratorDefaults.h"

#include <optional>

namespace PR {
/// Simple recursive random walker
template <bool ApplyRussianRoulette>
class Walker {
public:
	size_t MaxRayDepthHard;
	size_t MaxRayDepthSoft;
	float RussianRouletteRate = 0.80f;

	template <typename OnHitF, typename OnNonHitF>
	inline void traverse(RenderTileSession& session, Ray ray, const OnHitF& onHit, const OnNonHitF& onNonHit) const
	{
		for (size_t j = ray.IterationDepth; j < MaxRayDepthHard; ++j) {
			Vector3f pos;
			GeometryPoint gp;
			IEntity* entity		= nullptr;
			IMaterial* material = nullptr;
			if (!session.traceSingleRay(ray, pos, gp, entity, material)) {
				onNonHit(ray);
				break;
			} else {
				IntersectionPoint ip;
				ip.setForSurface(ray, pos, gp);
				std::optional<Ray> oray = onHit(ip, entity, material);
				if (oray.has_value())
					ray = oray.value();
				else
					break;
			}
		}
	}

	// Skip first trace -> useful for camera rays
	template <typename OnHitF, typename OnNonHitF>
	inline void traverse(RenderTileSession& session, const IntersectionPoint& ip, IEntity* entity, IMaterial* material,
						 const OnHitF& onHit, const OnNonHitF& onNonHit) const
	{
		std::optional<Ray> oray = onHit(ip, entity, material);
		if (oray.has_value())
			traverse(session, oray.value(), onHit, onNonHit);
	}

	template <typename OnHitF, typename OnSampleF, typename OnNonHitF>
	inline SpectralBlob traverseBSDF(RenderTileSession& session, const SpectralBlob& initial_weight, const Ray& initial_ray,
									 const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF_Base(
			session, initial_weight,
			[&](auto f1, auto f2) {
				traverse(session, initial_ray, f1, f2);
			},
			onHit, onSample, onNonHit);
	}

	// Skip first trace -> useful for camera rays
	template <typename OnHitF, typename OnSampleF, typename OnNonHitF>
	inline SpectralBlob traverseBSDF(RenderTileSession& session, const SpectralBlob& initial_weight,
									 const IntersectionPoint& ip, IEntity* entity, IMaterial* material,
									 const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF_Base(
			session, initial_weight,
			[&](auto f1, auto f2) {
				traverse(session, ip, entity, material, f1, f2);
			},
			onHit, onSample, onNonHit);
	}

	template <typename OnHitF, typename OnNonHitF>
	inline SpectralBlob traverseBSDFSimple(RenderTileSession& session, const SpectralBlob& initial_weight, const Ray& initial_ray,
										   const OnHitF& onHit, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF(
			session, initial_weight, initial_ray,
			onHit,
			[](SpectralBlob& weight, const MaterialSampleInput&, const MaterialSampleOutput& sout, IEntity*, IMaterial*) {
				weight *= sout.Weight;
			},
			onNonHit);
	}

private:
	template <typename TraverseF, typename OnHitF, typename OnSampleF, typename OnNonHitF>
	inline SpectralBlob traverseBSDF_Base(RenderTileSession& session, const SpectralBlob& initial_weight, const TraverseF& traverseFunc,
										  const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		auto& rnd			= session.tile()->random();
		SpectralBlob weight = initial_weight;
		traverseFunc(
			[&](const IntersectionPoint& ip, IEntity* entity, IMaterial* material) -> std::optional<Ray> {
				if (entity && material) {
					if (!onHit(weight, ip, entity, material))
						return {};

					// Russian roulette
					if constexpr (ApplyRussianRoulette) {
						if (ip.Ray.IterationDepth >= MaxRayDepthSoft) {
							constexpr float SCATTER_EPS = 1e-4f;

							const float russian_prob = rnd.getFloat();
							/*const float weight_f	 = weight.maxCoeff();*/
							const float scatProb = std::min<float>(1.0f, /*weight_f */ std::pow(RussianRouletteRate, ip.Ray.IterationDepth - MaxRayDepthSoft));
							if (russian_prob > scatProb || scatProb <= SCATTER_EPS)
								return {};

							weight /= scatProb;
						}
					}

					// Continue
					MaterialSampleInput sin;
					sin.Context		   = MaterialSampleContext::fromIP(ip);
					sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);
					sin.RND			   = rnd.get2D();

					MaterialSampleOutput sout;
					material->sample(sin, sout, session);

					onSample(weight, sin, sout, entity, material);

					if (weight.isZero(PR_EPSILON) || PR_UNLIKELY(sout.ForwardPDF_S[0] <= PR_EPSILON))
						return {};

					int rflags = RF_Bounce;
					if (material->isSpectralVarying())
						rflags |= RF_Monochrome;

					if (!material->hasDeltaDistribution())
						weight /= sout.ForwardPDF_S[0];

					return std::make_optional(ip.Ray.next(ip.P, sout.globalL(ip), ip.Surface.N,
														  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));

				} else { // Nothing found, abort
					return {};
				}
			},
			[&](const Ray& ray) { onNonHit(weight, ray); });
		return weight;
	}
};

} // namespace PR