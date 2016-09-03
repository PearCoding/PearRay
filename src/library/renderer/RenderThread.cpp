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
		mTile = mRenderer->getNextTile();

		while (mTile && !shouldStop())
		{
			for (uint32 y = mTile->sy(); y < mTile->ey() && !shouldStop(); ++y)
			{
				for (uint32 x = mTile->sx(); x < mTile->ex() && !shouldStop(); ++x)
				{
					mRenderer->render(&mContext, x, y, mTile->samplesRendered());
				}
			}

			mTile->inc();

			mTile->setWorking(false);
			mTile = mRenderer->getNextTile();
		}
	}
}