// IWYU pragma: private, include "renderer/StreamPipeline.h"
namespace PR {
inline void StreamPipeline::enqueueCameraRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
#ifndef PR_NO_RAY_STATISTICS
	mTile->statistics().add(RenderStatisticEntry::CameraRayCount);
	mTile->statistics().add(RenderStatisticEntry::PrimaryRayCount);
#endif
}

inline void StreamPipeline::enqueueLightRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
#ifndef PR_NO_RAY_STATISTICS
	mTile->statistics().add(RenderStatisticEntry::LightRayCount);
	mTile->statistics().add(RenderStatisticEntry::PrimaryRayCount);
#endif
}

inline void StreamPipeline::enqueueBounceRay(const Ray& ray)
{
	mWriteRayStream->addRay(ray);
#ifndef PR_NO_RAY_STATISTICS
	if (ray.Flags & RayFlag::Camera)
		mTile->statistics().add(RenderStatisticEntry::CameraRayCount);
	else if (ray.Flags & RayFlag::Light)
		mTile->statistics().add(RenderStatisticEntry::LightRayCount);
	mTile->statistics().add(RenderStatisticEntry::BounceRayCount);
#endif
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
