#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderManager.h"
#include "RenderTile.h"
#include "RenderSession.h"

#include "integrator/IIntegrator.h"

#include "ray/RayStream.h"
#include "shader/HitStream.h"
#include "registry/Registry.h"

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
	const size_t streamElemCount = mRenderer->renderManager()->registry()->getByGroup<uint64>(RG_RENDERER,
										 "stream/count",
										 1000000);

	RayStream rays(streamElemCount);
	HitStream hits(streamElemCount);

	size_t pass				= 0;
	IIntegrator* integrator = mRenderer->integrator();

	while (integrator->needNextPass(pass) && !shouldStop()) {
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop()) {
			integrator->onPass(RenderSession(mThreadIndex, mTile, &rays, &hits), pass);
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
