// IWYU pragma: private, include "renderer/StreamPipeline.h"
namespace PR {
inline void StreamPipeline::enqueueCameraRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
	mTile->statistics().addCameraRayCount();
}

inline void StreamPipeline::enqueueLightRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
	mTile->statistics().addLightRayCount();
}

inline void StreamPipeline::enqueueBounceRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
	mTile->statistics().addBounceRayCount();
}

inline Ray StreamPipeline::getTracedRay(size_t id) const
{
	return mReadRayStream->getRay(id);
}

inline const RayGroup& StreamPipeline::getRayGroup(size_t id) const
{
	return mGroupContainer.group(id);
}

inline bool StreamPipeline::hasShadingGroup() const { return mHitStream.hasNextGroup(); }

inline ShadingGroup StreamPipeline::popShadingGroup(const RenderTileSession& session)
{
	PR_ASSERT(hasShadingGroup(), "Trying to pop non existant shading group");

	return ShadingGroup(mHitStream.getNextGroup(), this, session);
}
} // namespace PR
