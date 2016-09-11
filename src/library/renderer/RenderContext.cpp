#include "RenderContext.h"
#include "sampler/Sampler.h"

namespace PR
{
	RenderContext::RenderContext(Renderer* renderer, RenderThread* thread, uint32 index) :
		mRenderer(renderer), mThread(thread), mIndex(index), mRandom(), mPixelSampler(nullptr)
	{
	}

	RenderContext::~RenderContext()
	{
		if (mPixelSampler)
		{
			delete mPixelSampler;
		}
	}
}