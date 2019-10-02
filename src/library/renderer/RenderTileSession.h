#pragma once

#include "Random.h"
#include "ray/RayPackage.h"
#include "ray/RayStream.h"
#include "renderer/RenderTileStatistics.h"
#include "renderer/RenderTile.h"
#include "trace/HitStream.h"
#include "trace/ShadowHit.h"

namespace PR {

class RenderTile;
class IEntity;
class IMaterial;
class ShadingPoint;

class PR_LIB RenderTileSession {
public:
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
					  RayStream* rayCoherentStream,
					  RayStream* rayIncoherentStream,
					  HitStream* hitStream);
	~RenderTileSession();

	inline uint32 thread() const
	{
		return mThread;
	}

	inline RenderTile* tile() const
	{
		return mTile;
	}

	inline HitStream* hitStream() const
	{
		return mHitStream;
	}

	IEntity* getEntity(uint32 id) const;

	bool handleCameraRays();

	inline void enqueueCoherentRay(const Ray& ray);
	inline bool enoughCoherentRaySpace(size_t requested = 1) const;
	inline Ray getCoherentRay(size_t id) const;
	inline void enqueueIncoherentRay(const Ray& ray);
	inline bool enoughIncoherentRaySpace(size_t requested = 1) const;
	inline Ray getIncoherentRay(size_t id) const;
	ShadowHit traceShadowRay(const Ray& ray) const;

	inline size_t maxBufferCount() const;

	template <typename Func>
	inline void handleHits(Func hitFunc);

	void pushFragment(uint32 pixelIndex, const ShadingPoint& pt) const;

private:
	void startShadingGroup(const ShadingGroup& grp, IEntity*& entity, IMaterial*& material);
	void endShadingGroup(const ShadingGroup& grp);

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	RayStream* mCoherentRayStream;
	RayStream* mIncoherentRayStream;
	HitStream* mHitStream;

	uint32 mCurrentX;
	uint32 mCurrentY;
};
} // namespace PR

#include "RenderTileSession.inl"