#pragma once

#include "RenderThreadStatistics.h"
#include "thread/Thread.h"

namespace PR {
class RenderContext;
class StreamPipeline;
class RenderTile;
class PR_LIB_CORE RenderThread : public Thread {
public:
	RenderThread(uint32 index, RenderContext* renderer);
	virtual ~RenderThread();

	inline RenderTile* currentTile() const
	{
		return mTile;
	}

	inline StreamPipeline* pipeline() const { return mPipeline.get(); }
	inline const RenderThreadStatistics& statistics() const { return mStatistics; }

protected:
	virtual void main();

private:
	const uint32 mThreadIndex;
	RenderContext* mRenderer;
	RenderTile* mTile;
	std::unique_ptr<StreamPipeline> mPipeline;
	RenderThreadStatistics mStatistics;
};
} // namespace PR
