#pragma once

#include "Random.h"
#include "entity/IEntity.h"
#include "geometry/GeometryPoint.h"
#include "ray/Ray.h"
#include "ray/RayGroupContainer.h"
#include "ray/RayStream.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "scene/Scene.h"
#include "shader/ShadingGroup.h"
#include "trace/HitStream.h"

namespace PR {
class RenderTile;
/// Encapsulates wavefront stream ray tracing pipeline
class PR_LIB_CORE StreamPipeline {
public:
	StreamPipeline(RenderContext* ctx);
	~StreamPipeline();

	void reset(RenderTile* tile);
	bool isFinished() const;
	void runPipeline();

	inline void enqueueCameraRay(const Ray& ray);
	inline void enqueueBounceRay(const Ray& ray);
	inline void enqueueLightRay(const Ray& ray);

	inline Ray getTracedRay(size_t id) const;
	inline const RayGroup& getRayGroup(size_t id) const;

	inline bool hasShadingGroup() const;
	inline ShadingGroup popShadingGroup(const RenderTileSession& session);

private:
	void fillWithCameraRays();

	RenderContext* mContext;
	RenderTile* mTile;
	std::unique_ptr<RayStream> mWriteRayStream;
	std::unique_ptr<RayStream> mReadRayStream;
	HitStream mHitStream;
	RayGroupContainer mGroupContainer;

	uint64 mCurrentVirtualPixelIndex;
	uint64 mCurrentPixelIndex;
	uint64 mMaxPixelCount;
};
} // namespace PR

#include "StreamPipeline.inl"