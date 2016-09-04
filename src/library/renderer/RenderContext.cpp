#include "RenderContext.h"
#include "Renderer.h"
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

	void RenderContext::render(uint32 x, uint32 y, uint32 sample, uint32 pass)
	{
		mRenderer->render(this, x, y, sample, pass);
	}
	
	RenderEntity* RenderContext::shoot(const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore)
	{
		return mRenderer->shoot(ray, collisionPoint, this, ignore);
	}

	RenderEntity* RenderContext::shootWithEmission(Spectrum& appliedSpec, const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore)
	{
		return mRenderer->shootWithEmission(appliedSpec, ray, collisionPoint, this, ignore);
	}
}