#include "RenderTileSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "scene/Scene.h"

namespace PR {
RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
									 RayStream* rayCoherentStream,
									 RayStream* rayIncoherentStream,
									 RayStream* rayShadowStream,
									 HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mCoherentRayStream(rayCoherentStream)
	, mIncoherentRayStream(rayIncoherentStream)
	, mShadowRayStream(rayShadowStream)
	, mHitStream(hitStream)
	, mCurrentPixel(0)
{
}

RenderTileSession::~RenderTileSession()
{
}

IEntity* RenderTileSession::getEntity(uint32 id) const
{
	return mTile->context().scene()->entities()[id].get();
}

bool RenderTileSession::handleCameraRays()
{
	const size_t endIndex = mTile->ex() * mTile->ey();
	if(mCurrentPixel >= endIndex)
		return false;

	for (; mCurrentPixel < endIndex; ++mCurrentPixel) {
		if (!enoughCoherentRaySpace(1))
			break;

		uint32 x = mCurrentPixel % mTile->width() + mTile->sx();
		uint32 y = mCurrentPixel / mTile->width() + mTile->sy();

		Ray ray = mTile->constructCameraRay(x, y, mTile->samplesRendered());
		enqueueCoherentRay(ray);
	}
	
	mTile->context().traceCoherentRays(*mCoherentRayStream, *mHitStream);

	return true;
}
} // namespace PR
