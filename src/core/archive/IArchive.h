#pragma once

#include "PR_Config.h"

namespace PR {
class RayStream;
class HitStream;
class BoundingBox;

struct Ray;
struct HitEntry;

/// An archive is a containerlike object with a bounding box. A scene is a special case of an archive
class PR_LIB_CORE IArchive {
public:
	/// Traces a stream of rays and produces a stream of hits
	virtual void traceRays(RayStream& rays, HitStream& hits) const = 0;
	/// Traces a single ray and returns true if something was hit. Information about the hit are updated if a hit was found
	virtual bool traceSingleRay(const Ray& ray, HitEntry& entry) const = 0;
	/// Traces a single ray and returns true if something lays within the given distance
	virtual bool traceShadowRay(const Ray& ray, float distance = PR_INF) const = 0;

	/// Bounding box containing all elements inside in world coordinates.
	virtual BoundingBox boundingBox() const = 0;
};
} // namespace PR