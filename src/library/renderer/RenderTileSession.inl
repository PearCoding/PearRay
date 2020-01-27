// IWYU pragma: private, include "renderer/RenderTileSession.h"
namespace PR {
inline void RenderTileSession::enqueueCameraRay(const Ray& ray)
{
	PR_ASSERT(enoughRaySpace(), "Check space requirement first!");
	mRayStream->addRay(ray);
	mTile->statistics().addCameraRayCount();
}

inline void RenderTileSession::enqueueLightRay(const Ray& ray)
{
	PR_ASSERT(enoughRaySpace(), "Check space requirement first!");
	mRayStream->addRay(ray);
	mTile->statistics().addLightRayCount();
}

inline bool RenderTileSession::enoughRaySpace(size_t requested) const
{
	return mRayStream->enoughSpace(requested);
}

inline Ray RenderTileSession::getRay(size_t id) const
{
	return mRayStream->getRay(id);
}

inline size_t RenderTileSession::maxBufferCount() const
{
	return mRayStream->maxSize();
}

template <typename Func1, typename Func2>
inline void RenderTileSession::handleHits(Func1 nonhitFunc, Func2 hitFunc)
{
	mTile->context()->scene()->traceRays(
		*mRayStream,
		*mHitStream,
		nonhitFunc);

	mHitStream->setup(mTile->context()->settings().sortHits);
	while (mHitStream->hasNextGroup()) {
		ShadingGroup grp	= mHitStream->getNextGroup();
		IEntity* entity		= nullptr;
		IMaterial* material = nullptr;
		startShadingGroup(grp, entity, material);

		for (size_t i = grp.Start; i <= grp.End; ++i) {
			HitEntry entry = mHitStream->get(i);
			Ray ray		   = mRayStream->getRay(entry.RayID);

			if (!material)
				pushFeedbackFragment(OF_MissingMaterial, ray);

			GeometryPoint pt;
			entity->provideGeometryPoint(ray.Direction, entry.PrimitiveID,
										 Vector3f(entry.Parameter[0], entry.Parameter[1], entry.Parameter[2]), pt);

			hitFunc(entry, ray, pt, entity, material);
		}
		endShadingGroup(grp);
	}
}
} // namespace PR
