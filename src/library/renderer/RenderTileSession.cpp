#include "RenderTileSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/OutputBuffer.h"
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
	, mCurrentX(0)
	, mCurrentY(0)
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
	const uint32 w = mTile->width();
	const uint32 h = mTile->height();

	if (mCurrentY >= h)
		return false;

	mCoherentRayStream->reset();

	for (; mCurrentY < h; ++mCurrentY) {
		const uint32 fy = mCurrentY + mTile->sy();

		// Allow continuation
		if (mCurrentX >= w)
			mCurrentX = 0;

		for (; mCurrentX < w; ++mCurrentX) {
			if (!enoughCoherentRaySpace(1))
				break;

			const uint32 fx = mCurrentX + mTile->sx();

			Ray ray		   = mTile->constructCameraRay(fx, fy,
												   mTile->samplesRendered());
			ray.PixelIndex = fy * mTile->context().width() + fx;

			enqueueCoherentRay(ray);
		}
	}

	/*{
		std::stringstream sstream;
		sstream << "rays_t" << mTile->index()
				<< "_p" << (mCurrentY * w + mCurrentX)
				<< "_s" << mTile->samplesRendered() << ".rdmp";
		mCoherentRayStream->dump(sstream.str());
	}*/

	mTile->context().scene()->traceCoherentRays(
		*mCoherentRayStream,
		*mHitStream,
		[&](const Ray& ray) {
			mTile->statistics().addBackgroundHitCount();
			mTile->context().output()->pushBackgroundFragment(ray.PixelIndex,
															  ray.WavelengthIndex);
		});

	return true;
}

void RenderTileSession::startShadingGroup(const ShadingGroup& grp,
										  IEntity*& entity, IMaterial*& material)
{
	entity   = mTile->context().getEntity(grp.EntityID);
	material = mTile->context().getMaterial(grp.MaterialID);
}

void RenderTileSession::endShadingGroup(const ShadingGroup& grp)
{
}

ShadowHit RenderTileSession::traceShadowRay(const Ray& ray) const
{
	mTile->statistics().addShadowRayCount();
	return mTile->context().scene()->traceShadowRay(ray);
}

void RenderTileSession::pushFragment(const uint32 pixelIndex, const ShadingPoint& pt) const
{
	mTile->context().output()->pushFragment(pixelIndex, pt);
}
} // namespace PR
