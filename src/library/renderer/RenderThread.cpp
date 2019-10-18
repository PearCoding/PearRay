#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderTileSession.h"
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
	, mStreamElementCount(100000)
{
	PR_ASSERT(renderer, "RenderThread needs valid renderer");
}

void RenderThread::main()
{
	RayStream rays(mStreamElementCount);
	HitStream hits(mStreamElementCount);

	size_t pass		= 0;
	auto integrator = mRenderer->integrator();

	while (integrator->needNextPass(pass) && !shouldStop()) {
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop()) {
			RenderTileSession session(mThreadIndex, mTile,
									  &rays, &hits);
			mStatistics.addTileCount();

			integrator->onPass(session, pass);
			mTile->inc();

			mTile->setWorking(false);
			mTile = mRenderer->getNextTile();
		}

		if (shouldStop())
			break;

		mRenderer->waitForNextPass();

		pass++;
	}
}
} // namespace PR
