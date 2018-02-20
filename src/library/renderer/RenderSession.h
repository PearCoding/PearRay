#pragma once

#include "Random.h"
#include "renderer/RenderStatistics.h"

namespace PR {

class RenderTile;
class PR_LIB RenderSession {
public:
	RenderSession(uint32 threadIndex, RenderTile* tile);
	~RenderSession();

	inline uint32 thread() const
	{
		return mThread;
	}

	inline RenderTile* tile() const
	{
		return mTile;
	}

private:
	uint32 mThread;
	RenderTile* mTile;
};
}
