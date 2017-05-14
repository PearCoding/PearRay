#include "RenderThreadContext.h"
#include "sampler/Sampler.h"

namespace PR
{
	RenderThreadContext::RenderThreadContext(RenderContext* renderer, RenderThread* thread, uint32 index) :
		mRenderer(renderer), mThread(thread), mIndex(index), mRandom(renderer->settings().seed() + index),
		mAASampler(nullptr), mLensSampler(nullptr), mTimeSampler(nullptr), mSpectralSampler(nullptr)
	{
	}

	RenderThreadContext::~RenderThreadContext()
	{
		if (mAASampler)
			delete mAASampler;
		if (mLensSampler)
			delete mLensSampler;
		if (mTimeSampler)
			delete mTimeSampler;
		if (mSpectralSampler)
			delete mSpectralSampler;
	}
}
