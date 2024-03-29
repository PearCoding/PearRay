#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"

#include "vcm/Defaults.h"

#include <optional>

namespace PR {
/// Simple recursive random walker
template <bool ApplyRussianRoulette>
class Walker {
public:
	size_t MaxRayDepthHard	  = 64;
	size_t MaxRayDepthSoft	  = 4;
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
	inline SpectralBlob traverseBSDF(Random& rnd, RenderTileSession& session, const SpectralBlob& initial_weight, const Ray& initial_ray,
									 const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF_Base(
			rnd, session, initial_weight,
			[&](auto f1, auto f2) {
				traverse(session, initial_ray, f1, f2);
			},
			onHit, onSample, onNonHit);
	}

	// Skip first trace -> useful for camera rays
	template <typename OnHitF, typename OnSampleF, typename OnNonHitF>
	inline SpectralBlob traverseBSDF(Random& rnd, RenderTileSession& session, const SpectralBlob& initial_weight,
									 const IntersectionPoint& ip, IEntity* entity, IMaterial* material,
									 const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF_Base(
			rnd, session, initial_weight,
			[&](auto f1, auto f2) {
				traverse(session, ip, entity, material, f1, f2);
			},
			onHit, onSample, onNonHit);
	}

	template <typename OnHitF, typename OnNonHitF>
	inline SpectralBlob traverseBSDFSimple(Random& rnd, RenderTileSession& session, const SpectralBlob& initial_weight, const Ray& initial_ray,
										   const OnHitF& onHit, const OnNonHitF& onNonHit) const
	{
		return traverseBSDF(
			rnd, session, initial_weight, initial_ray,
			onHit,
			[](SpectralBlob& weight, const MaterialSampleInput&, const MaterialSampleOutput& sout, IEntity*, IMaterial*) {
				weight *= sout.IntegralWeight;
			},
			onNonHit);
	}

private:
	template <typename TraverseF, typename OnHitF, typename OnSampleF, typename OnNonHitF>
	inline SpectralBlob traverseBSDF_Base(Random& rnd, RenderTileSession& session, const SpectralBlob& initial_weight, const TraverseF& traverseFunc,
										  const OnHitF& onHit, const OnSampleF& onSample, const OnNonHitF& onNonHit) const
	{
		SpectralBlob weight = initial_weight;
		traverseFunc(
			[&](const IntersectionPoint& ip, IEntity* entity, IMaterial* material) -> std::optional<Ray> {
				if (PR_UNLIKELY(!entity))
					return {};

				if (!onHit(weight, ip, entity, material))
					return {};

				if (PR_UNLIKELY(!material))
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
				MaterialSampleInput sin(rnd);
				sin.Context		   = MaterialSampleContext::fromIP(ip);
				sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);

				MaterialSampleOutput sout;
				material->sample(sin, sout, session);

				onSample(weight, sin, sout, entity, material);

				if (weight.isZero(PR_EPSILON) || PR_UNLIKELY(sout.PDF_S[0] <= PR_EPSILON))
					return {};

				if (sout.isHeroCollapsing())
					weight *= SpectralBlobUtils::HeroOnly();

				RayFlags rflags = RayFlag::Bounce;
				if (sout.isHeroCollapsing())
					rflags |= RayFlag::Monochrome;

				if (!sout.isDelta())
					weight /= sout.PDF_S[0];

				return std::make_optional(ip.Ray.next(ip.P, sout.globalL(ip), ip.Surface.N,
													  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
			},
			[&](const Ray& ray) { onNonHit(weight, ray); });
		return weight;
	}
};

} // namespace PR