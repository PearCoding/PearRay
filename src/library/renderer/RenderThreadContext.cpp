#include "RenderThreadContext.h"
#include "sampler/Sampler.h"

namespace PR
{
	RenderThreadContext::RenderThreadContext(RenderContext* renderer, RenderThread* thread, uint32 index) :
		mRenderer(renderer), mThread(thread), mIndex(index), mRandom(), mPixelSampler(nullptr)
	{
	}

	RenderThreadContext::~RenderThreadContext()
	{
		if (mPixelSampler)
		{
			delete mPixelSampler;
		}
	}
}
