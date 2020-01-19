#include "RenderThread.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderTileSession.h"
#include "buffer/OutputBufferBucket.h"
#include "integrator/IIntegrator.h"
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

void RenderThread::main()
{
	std::stringstream namestream;
	namestream << "Worker " << mThreadIndex;
	PR_PROFILE_THREAD(namestream.str());

	RayStream rays(mRenderer->settings().maxParallelRays);
	HitStream hits(mRenderer->settings().maxParallelRays);

	uint32 pass		= 0;
	auto integrator = mRenderer->integrator();

	std::shared_ptr<OutputBufferBucket> bucket = mRenderer->output()->createBucket(mRenderer->maxTileSize());

	integrator->onThreadStart(mRenderer, mThreadIndex);
	while (integrator->needNextPass(pass) && !shouldStop()) {
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop()) {
			RenderTileSession session(mThreadIndex, mTile,
									  bucket,
									  &rays, &hits);

			mTile->incIteration();
			mStatistics.addTileCount();

			bucket->clear(true);
			integrator->onPass(session, pass);
			if (shouldStop())
				break;
			mRenderer->output()->mergeBucket(mTile->start(), bucket);

			mTile->setWorking(false);
			mTile = mRenderer->getNextTile();
		}

		if (shouldStop())
			break;

		mRenderer->waitForNextPass();

		pass++;
	}
	integrator->onThreadEnd(mRenderer, mThreadIndex);
}
} // namespace PR
