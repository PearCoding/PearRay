#include "RenderThread.h"
#include "Platform.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderTileSession.h"
#include "StreamPipeline.h"
#include "integrator/IIntegrator.h"
#include "output/LocalOutputQueue.h"
#include "output/LocalOutputSystem.h"
#include "output/OutputSystem.h"
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

	auto outputSystem = mRenderer->output();
	auto integrator	  = mRenderer->integrator()->createThreadInstance(mRenderer, mThreadIndex);
	auto queue		  = std::make_shared<LocalOutputQueue>(outputSystem.get(), mPipeline.get(), QUEUE_SIZE, QUEUE_THRESHOLD);

	integrator->onStart();
	for (mTile = mRenderer->getNextTile(this);
		 mTile && !shouldStop();
		 mTile = mRenderer->getNextTile(this)) {

		auto localSystem = outputSystem->createLocal(mTile, mTile->viewSize());
		RenderTileSession session(mThreadIndex, mTile, pipeline(), queue, localSystem);

		localSystem->clear(true);
		mPipeline->reset(mTile);
		integrator->onTile(session);
		if (PR_UNLIKELY(shouldStop())) {
			mTile->release();
			break;
		}
		queue->commitAndFlush(localSystem.get());
		outputSystem->mergeLocal(mTile->start(), localSystem, mRenderer->currentIteration().Iteration + 1);

		mStatistics.addTileCount();
		mTile->release();
	}
	integrator->onEnd();
}
} // namespace PR
