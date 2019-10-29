// IWYU pragma: private, include "renderer/RenderTileSession.h"
namespace PR {
inline void RenderTileSession::enqueueRay(const Ray& ray)
{
	PR_ASSERT(enoughRaySpace(), "Check space requirement first!");
	mRayStream->addRay(ray);
	mTile->statistics().addRayCount();
}

inline bool RenderTileSession::enoughRaySpace(size_t requested) const
{
	return mRayStream->enoughSpace(requested);
}

inline void RenderTileSession::sendRay(size_t id, const Ray& ray)
{
	mRayStream->setRay(id, ray);
	mTile->statistics().addRayCount();
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

	mHitStream->sort();
	while (mHitStream->hasNextGroup()) {
		ShadingGroup grp	= mHitStream->getNextGroup();
		IEntity* entity		= nullptr;
		IMaterial* material = nullptr;
		startShadingGroup(grp, entity, material);

		for (uint32 i = grp.Start; i <= grp.End; ++i) {
			HitEntry entry = mHitStream->get(i);
			Ray ray		   = mRayStream->getRay(entry.SessionRayID);

			if (!material)
				pushFeedbackFragment(ray, OF_MissingMaterial);

			GeometryPoint pt;
			entity->provideGeometryPoint(entry.PrimitiveID, entry.UV[0], entry.UV[1], pt);

			hitFunc(entry, ray, pt, entity, material);
		}
		endShadingGroup(grp);
	}
}
} // namespace PR
