#pragma once

#include "Random.h"
#include "renderer/RenderStatistics.h"

namespace PR {

class HitStream;
class RayStream;
class RenderTile;
class IEntity;

class PR_LIB RenderTileSession {
public:
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
		RayStream* rayStream, HitStream* hitStream);
	~RenderTileSession();

	inline uint32 thread() const
	{
		return mThread;
	}

	inline RenderTile* tile() const
	{
		return mTile;
	}

	inline RayStream* rayStream() const
	{
		return mRayStream;
	}

	inline HitStream* hitStream() const
	{
		return mHitStream;
	}

	IEntity* getEntity(uint32 id) const;
private:
	uint32 mThread;
	RenderTile* mTile;
	RayStream* mRayStream;
	HitStream* mHitStream;
};
}
