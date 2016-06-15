#include "RenderContext.h"
#include "Renderer.h"

#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/UniformSampler.h"

namespace PR
{
	RenderContext::RenderContext(Renderer* renderer, RenderThread* thread, uint32 index) :
		mRenderer(renderer), mThread(thread), mPixelSampler(nullptr), mIndex(index)
	{
		/* Setup samplers */
		switch (mRenderer->settings().pixelSampler())
		{
		case SM_Random:
			mPixelSampler = new RandomSampler(mRenderer->random());
			break;
		case SM_Uniform:
			mPixelSampler = new UniformSampler(mRenderer->random(), mRenderer->settings().maxPixelSampleCount());
			break;
		default:
		case SM_Jitter:
			mPixelSampler = new StratifiedSampler(mRenderer->random(), mRenderer->settings().maxPixelSampleCount());
			break;
		case SM_MultiJitter:
			mPixelSampler = new MultiJitteredSampler(mRenderer->random(), mRenderer->settings().maxPixelSampleCount());
			break;
		}
	}

	RenderContext::~RenderContext()
	{
		if (mPixelSampler)
		{
			delete mPixelSampler;
			mPixelSampler = nullptr;
		}
	}

	RenderEntity* RenderContext::shoot(const Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore)
	{
		return mRenderer->shoot(ray, collisionPoint, this, ignore);
	}

	RenderEntity* RenderContext::shootWithApply(Spectrum& appliedSpec, const Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore)
	{
		return mRenderer->shootWithApply(appliedSpec, ray, collisionPoint, this, ignore);
	}
}