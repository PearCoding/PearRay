#include "RenderThread.h"
#include "Renderer.h"
#include "RenderTile.h"

#include "integrator/Integrator.h"

namespace PR
{
	RenderThread::RenderThread(Renderer* renderer, uint32 index) :
		Thread(), mRenderer(renderer), mTile(nullptr), mContext(renderer, this, index)
	{
		PR_ASSERT(renderer);
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

			integrator->onPostPass(&mContext, pass);
			
			if(shouldStop())
				break;
			
			mRenderer->waitForNextPass();

			pass++;
		}
		integrator->onThreadEnd(&mContext);
	}
}