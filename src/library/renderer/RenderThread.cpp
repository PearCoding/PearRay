#include "RenderThread.h"
#include "Renderer.h"

namespace PR
{
	RenderThread::RenderThread(size_t sx, size_t sy, size_t ex, size_t ey, Renderer* renderer) :
		mStartX(sx), mStartY(sy), mEndX(ex), mEndY(ey),
		mRenderer(renderer)
	{
		PR_ASSERT(renderer);
	}

	void RenderThread::main()
	{
		mPixelsRendered = 0;
		for (uint32 y = mStartY; y < mEndY && !shouldStop(); ++y)
		{
			for (uint32 x = mStartX; x < mEndX && !shouldStop(); ++x)
			{
				mRenderer->render(x, y);
				mPixelsRendered++;
			}
		}
	}

	size_t RenderThread::pixelsRendered() const
	{
		return mPixelsRendered;
	}
}