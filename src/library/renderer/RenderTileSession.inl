namespace PR {
inline void RenderTileSession::enqueueCoherentRay(const Ray& ray)
{
	PR_ASSERT(enoughCoherentRaySpace(), "Check space requirement first!");
	mCoherentRayStream->add(ray);
	mTile->statistics().addCoherentRayCount();
}

inline bool RenderTileSession::enoughCoherentRaySpace(size_t requested) const
{
	return mCoherentRayStream->enoughSpace(requested);
}

inline Ray RenderTileSession::getCoherentRay(size_t id) const
{
	return mCoherentRayStream->getRay(id);
}

inline void RenderTileSession::enqueueIncoherentRay(const Ray& ray)
{
	PR_ASSERT(enoughIncoherentRaySpace(), "Check space requirement first!");
	mIncoherentRayStream->add(ray);
	mTile->statistics().addIncoherentRayCount();
}

inline bool RenderTileSession::enoughIncoherentRaySpace(size_t requested) const
{
	return mIncoherentRayStream->enoughSpace(requested);
}

inline Ray RenderTileSession::getIncoherentRay(size_t id) const
{
	return mIncoherentRayStream->getRay(id);
}

inline size_t RenderTileSession::maxBufferCount() const
{
	return mCoherentRayStream->maxSize();
}

template <typename Func>
inline void RenderTileSession::handleHits(Func hitFunc)
{
	while (mHitStream->hasNextGroup()) {
		ShadingGroup grp = mHitStream->getNextGroup();
		IEntity* entity;
		IMaterial* material;
		startShadingGroup(grp, entity, material);

		for (uint32 i = grp.Start; i < grp.End; ++i) {
			HitEntry entry;
			entry.Flags		  = mHitStream->flags(i);
			entry.RayID		  = mHitStream->rayID(i);
			entry.MaterialID  = grp.MaterialID;
			entry.EntityID	= grp.EntityID;
			entry.PrimitiveID = mHitStream->primitiveID(i);
			entry.UV[0]		  = mHitStream->uv(0, i);
			entry.UV[1]		  = mHitStream->uv(1, i);
			hitFunc(entry, entity, material);
		}
		endShadingGroup(grp);
	}
}
} // namespace PR
