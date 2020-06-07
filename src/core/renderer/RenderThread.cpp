#include "RenderThread.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderTileSession.h"
#include "buffer/FrameBufferBucket.h"
#include "buffer/FrameBufferSystem.h"
#include "integrator/IIntegrator.h"
#include "output/OutputQueue.h"
#include "ray/RayStream.h"
#include "trace/HitStream.h"

#include "Logger.h"

namespace PR {
RenderThread::RenderThread(uint32 index, RenderContext* renderer)
	: Thread()
	, mThreadIndex(index)
	, mRenderer(renderer)
	, mTile(nullptr)
{
	PR_ASSERT(renderer, "RenderThread needs valid renderer");
}

constexpr size_t QUEUE_SIZE		 = 1024;
constexpr size_t QUEUE_THRESHOLD = 950;

void RenderThread::main()
{
	std::stringstream namestream;
	namestream << "Worker " << mThreadIndex;
	PR_PROFILE_THREAD(namestream.str());

	auto integrator					   = mRenderer->integrator()->createThreadInstance(mRenderer, mThreadIndex);
	std::shared_ptr<OutputQueue> queue = std::make_shared<OutputQueue>(QUEUE_SIZE, QUEUE_THRESHOLD);
	auto bucket						   = mRenderer->output()->createBucket(mRenderer->maxTileSize());

	mTile = nullptr;
	mTile = mRenderer->getNextTile();

	integrator->onStart();
	while (mTile && !shouldStop()) {
		RenderTileSession session(mThreadIndex, mTile, queue, bucket);

		mTile->incIteration();
		mStatistics.addTileCount();

		bucket->clear(true);
		integrator->onTile(session);
		if (PR_UNLIKELY(shouldStop()))
			break;
		queue->commitAndFlush(bucket.get());
		mRenderer->output()->mergeBucket(mTile->start(), bucket);

		mTile->release();
		mTile = nullptr;
		mTile = mRenderer->getNextTile();
	}
	integrator->onEnd();
}
} // namespace PR
