#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"

#include "integrator/Integrator.h"

#include "Logger.h"
namespace PR {
RenderThread::RenderThread(RenderContext* renderer)
	: Thread()
	, mRenderer(renderer)
	, mTile(nullptr)
{
	PR_ASSERT(renderer, "RenderThread needs valid renderer");
}

void RenderThread::main()
{
	size_t pass			   = 0;
	Integrator* integrator = mRenderer->integrator();

	while (integrator->needNextPass(pass) && !shouldStop()) {
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop()) {
			integrator->onPass(mTile, pass);
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
}
