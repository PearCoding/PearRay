namespace PR {
inline void RenderTileSession::enqueueCoherentRay(const Ray& ray)
{
	PR_ASSERT(enoughCoherentRaySpace(), "Check space requirement first!");
	mCoherentRayStream->add(ray);
}

inline bool RenderTileSession::enoughCoherentRaySpace(size_t requested) const
{
	return mCoherentRayStream->enoughSpace(requested);
}

inline void RenderTileSession::enqueueIncoherentRay(const Ray& ray)
{
	PR_ASSERT(enoughIncoherentRaySpace(), "Check space requirement first!");
	mIncoherentRayStream->add(ray);
}

inline bool RenderTileSession::enoughIncoherentRaySpace(size_t requested) const
{
	return mIncoherentRayStream->enoughSpace(requested);
}

inline void RenderTileSession::enqueueShadowRay(const Ray& ray)
{
	PR_ASSERT(enoughShadowRaySpace(), "Check space requirement first!");
	mShadowRayStream->add(ray);
}

inline bool RenderTileSession::enoughShadowRaySpace(size_t requested) const
{
	return mShadowRayStream->enoughSpace(requested);
}

inline size_t RenderTileSession::maxBufferCount() const
{
	return mCoherentRayStream->maxSize();
}
} // namespace PR
