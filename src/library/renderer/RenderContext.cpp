#include "RenderContext.h"
#include "Renderer.h"

namespace PR
{
	RenderContext::RenderContext(Renderer* renderer, RenderThread* thread, uint32 index) :
		mRenderer(renderer), mThread(thread), mIndex(index)
	{

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