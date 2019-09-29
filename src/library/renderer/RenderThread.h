#pragma once

#include "thread/Thread.h"

namespace PR {
class RenderContext;
class RenderTile;
class PR_LIB RenderThread : public Thread {
public:
	RenderThread(uint32 index, RenderContext* renderer);

	inline RenderTile* currentTile() const
	{
		return mTile;
	}

protected:
	virtual void main();

private:
	uint32 mThreadIndex;
	RenderContext* mRenderer;
	RenderTile* mTile;

	size_t mStreamElementCount;
};
}
