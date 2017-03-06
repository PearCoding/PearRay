#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"

#include "integrator/Integrator.h"

#include "Logger.h"
namespace PR
{
	RenderThread::RenderThread(RenderContext* renderer, uint32 index) :
		Thread(), mRenderer(renderer), mTile(nullptr), mContext(renderer, this, index)
	{
		PR_ASSERT(renderer, "RenderThread needs valid renderer");
	}

	void RenderThread::main()
	{
		size_t pass = 0;
		Integrator* integrator = mRenderer->integrator();

		integrator->onThreadStart(&mContext);
		while(integrator->needNextPass(pass) && !shouldStop())
		{
			integrator->onPrePass(&mContext, pass);

			mTile = mRenderer->getNextTile();

			while (mTile && !shouldStop())
			{
				integrator->onPass(mTile, &mContext, pass);
				mTile->inc();

				mTile->setWorking(false);
				mTile = mRenderer->getNextTile();
			}

			if(shouldStop())
				break;
			
			integrator->onPostPass(&mContext, pass);

			if(shouldStop())
				break;

			mRenderer->waitForNextPass();

			pass++;
		}
		integrator->onThreadEnd(&mContext);
	}
}
