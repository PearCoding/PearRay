#include "RenderThread.h"
#include "Platform.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderTileSession.h"
#include "StreamPipeline.h"
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

	mPipeline = std::make_unique<StreamPipeline>(renderer);
}

RenderThread::~RenderThread()
{
}

constexpr size_t QUEUE_SIZE		 = 1024;
constexpr size_t QUEUE_THRESHOLD = 950;

void RenderThread::main()
{
	std::stringstream namestream;
	namestream << "Worker " << mThreadIndex;
	PR_PROFILE_THREAD(namestream.str());

	setupFloatingPointEnvironment();

	auto integrator					   = mRenderer->integrator()->createThreadInstance(mRenderer, mThreadIndex);
	std::shared_ptr<OutputQueue> queue = std::make_shared<OutputQueue>(QUEUE_SIZE, QUEUE_THRESHOLD);
	auto bucket						   = mRenderer->output()->createBucket(mRenderer->maxTileSize());

	integrator->onStart();
	for (mTile = mRenderer->getNextTile();
		 mTile && !shouldStop();
		 mTile = mRenderer->getNextTile()) {

		RenderTileSession session(mThreadIndex, mTile, pipeline(), queue, bucket);

		mTile->incIteration();
		mStatistics.addTileCount();

		bucket->clear(true);
		mPipeline->reset(mTile);
		integrator->onTile(session);
		if (PR_UNLIKELY(shouldStop())) {
			mTile->release();
			break;
		}
		queue->commitAndFlush(bucket.get());
		mRenderer->output()->mergeBucket(mTile->start(), bucket);

		mTile->release();
	}
	integrator->onEnd();
}
} // namespace PR
