#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"

#include "Defaults.h"

#include <optional>

namespace PR {
/// Simple recursive random walker
class Walker {
private:
	size_t mMaxRayDepth;

public:
	inline explicit Walker(size_t maxDepth)
		: mMaxRayDepth(maxDepth)
	{
	}
	
	template <typename OnHitF, typename OnNonHitF>
	inline void traverse(RenderTileSession& session, Ray ray, const OnHitF& onHit, const OnNonHitF& onNonHit) const
	{
		for (size_t j = ray.IterationDepth; j < mMaxRayDepth; ++j) {
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
};

} // namespace PR