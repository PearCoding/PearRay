#pragma once

#include "ShadingGroupBlock.h"
#include "ray/Ray.h"

namespace PR {
class RenderTile;
class RenderTileSession;
class StreamPipeline;
class IEntity;
class IMaterial;
struct GeometryPoint;
class ShadingPoint;
struct HitEntry;

class PR_LIB_CORE ShadingGroup {
public:
	ShadingGroup(const ShadingGroupBlock& blck, const StreamPipeline* pipeline, const RenderTileSession& session);
	~ShadingGroup();

	inline bool isBackground() { return mBlock.EntityID == PR_INVALID_ID; }
	inline size_t size() const { return mBlock.size(); }

	inline IEntity* entity() const { return mEntity; }

	// Single (slow) access interface
	void extractHitEntry(size_t i, HitEntry& entry) const;
	void extractRay(size_t i, Ray& ray) const;
	void extractGeometryPoint(size_t i, GeometryPoint& pt) const;
	void computeShadingPoint(size_t i, ShadingPoint& spt) const;

private:
	ShadingGroupBlock mBlock;
	const StreamPipeline* mPipeline;

	// Constructed entries
	IEntity* mEntity;
};

} // namespace PR
