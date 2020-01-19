#pragma once

#include "RenderThreadStatistics.h"
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

	inline const RenderThreadStatistics& statistics() const { return mStatistics; }

protected:
	virtual void main();

private:
	const uint32 mThreadIndex;
	RenderContext* mRenderer;
	RenderTile* mTile;
	RenderThreadStatistics mStatistics;
};
} // namespace PR
