#include "RenderThread.h"
#include "Renderer.h"

namespace PR
{
	RenderThread::RenderThread(Renderer* renderer, uint32 index) :
		mRenderer(renderer), mContext(renderer, this, index)
	{
		PR_ASSERT(renderer);
	}

	void RenderThread::main()
	{
		mPixelsRendered = 0;
		uint32 sx;
		uint32 sy;
		uint32 ex; 
		uint32 ey;

		while (mRenderer->getNextTile(sx, sy, ex, ey) && !shouldStop())
		{
			for (uint32 y = sy; y < ey && !shouldStop(); ++y)
			{
				for (uint32 x = sx; x < ex && !shouldStop(); ++x)
				{
					mRenderer->render(&mContext, x, y);
					mPixelsRendered++;
				}
			}
		}
	}

	size_t RenderThread::pixelsRendered() const
	{
		return mPixelsRendered;
	}
}