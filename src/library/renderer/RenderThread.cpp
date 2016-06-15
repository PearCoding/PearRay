#include "RenderThread.h"
#include "Renderer.h"
#include "RenderTile.h"

namespace PR
{
	RenderThread::RenderThread(Renderer* renderer, uint32 index) :
		mRenderer(renderer), mTile(nullptr), mContext(renderer, this, index)
	{
		PR_ASSERT(renderer);
	}

	void RenderThread::main()
	{
		mSamplesRendered = 0;
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop())
		{
			for (uint32 y = mTile->sy(); y < mTile->ey() && !shouldStop(); ++y)
			{
				for (uint32 x = mTile->sx(); x < mTile->ex() && !shouldStop(); ++x)
				{
					mRenderer->render(&mContext, x, y, mTile->samplesRendered());
					if (mRenderer->settings().isProgressive())
						mSamplesRendered++;
					else
						mSamplesRendered += mRenderer->settings().maxPixelSampleCount();
				}
			}

			if(mRenderer->settings().isProgressive())
				mTile->inc();

			mTile->setWorking(false);
			mTile = mRenderer->getNextTile();
		}
	}

	size_t RenderThread::samplesRendered() const
	{
		return mSamplesRendered;
	}
}