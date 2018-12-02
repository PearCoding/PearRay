#pragma once

#include "Random.h"
#include "ray/RayPackage.h"
#include "ray/RayStream.h"
#include "renderer/RenderStatistics.h"

namespace PR {

class HitStream;
class RenderTile;
class IEntity;

class PR_LIB RenderTileSession {
public:
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
					  RayStream* rayCoherentStream,
					  RayStream* rayIncoherentStream,
					  RayStream* rayShadowStream,
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
	inline void enqueueIncoherentRay(const Ray& ray);
	inline bool enoughIncoherentRaySpace(size_t requested = 1) const;
	inline void enqueueShadowRay(const Ray& ray);
	inline bool enoughShadowRaySpace(size_t requested = 1) const;

	inline size_t maxBufferCount() const;

private:
	uint32 mThread;
	RenderTile* mTile;
	RayStream* mCoherentRayStream;
	RayStream* mIncoherentRayStream;
	RayStream* mShadowRayStream;
	HitStream* mHitStream;

	uint32 mCurrentPixel;
};
} // namespace PR

#include "RenderTileSession.inl"