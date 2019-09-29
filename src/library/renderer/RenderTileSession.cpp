#include "RenderTileSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "scene/Scene.h"

namespace PR {
RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
									 RayStream* rayCoherentStream,
									 RayStream* rayIncoherentStream,
									 HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mCoherentRayStream(rayCoherentStream)
	, mIncoherentRayStream(rayIncoherentStream)
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
	if (mCurrentPixel >= endIndex)
		return false;

	mCoherentRayStream->reset();

	for (; mCurrentPixel < endIndex; ++mCurrentPixel) {
		if (!enoughCoherentRaySpace(1))
			break;

		uint32 x = mCurrentPixel % mTile->width() + mTile->sx();
		uint32 y = mCurrentPixel / mTile->width() + mTile->sy();

		Ray ray = mTile->constructCameraRay(x, y, mTile->samplesRendered());
		enqueueCoherentRay(ray);
	}

	mTile->context().scene()->traceCoherentRays(*mCoherentRayStream, *mHitStream,
												[&](const Ray& ray) {
													mTile->statistics().addBackgroundHitCount();
												});

	return true;
}

void RenderTileSession::startShadingGroup(const ShadingGroup& grp,
										  IEntity*& entity, IMaterial*& material)
{
	/*entity   = mTile->context().renderManager()->entityManager()->getObject(grp.EntityID).get();
	material = mTile->context().renderManager()->materialManager()->getObject(grp.MaterialID).get();*/
}

void RenderTileSession::endShadingGroup(const ShadingGroup& grp)
{
}
} // namespace PR
