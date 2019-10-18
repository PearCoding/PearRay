#include "RenderTileSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/OutputBuffer.h"
#include "scene/Scene.h"
#include "shader/ShadingPoint.h"

namespace PR {
RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
									 RayStream* rayStream,
									 HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mRayStream(rayStream)
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
	return id < mTile->context()->scene()->entities().size()
			   ? mTile->context()->scene()->entities()[id].get()
			   : nullptr;
}

IMaterial* RenderTileSession::getMaterial(uint32 id) const
{
	return id < mTile->context()->scene()->materials().size()
			   ? mTile->context()->scene()->materials()[id].get()
			   : nullptr;
}

IEmission* RenderTileSession::getEmission(uint32 id) const
{
	return id < mTile->context()->scene()->emissions().size()
			   ? mTile->context()->scene()->emissions()[id].get()
			   : nullptr;
}

bool RenderTileSession::handleCameraRays()
{
	const uint32 w = mTile->width();
	const uint32 h = mTile->height();

	if (mCurrentY >= h)
		return false;

	mRayStream->reset();

	for (; mCurrentY < h; ++mCurrentY) {
		const uint32 fy = mCurrentY + mTile->sy();

		// Allow continuation
		if (mCurrentX >= w)
			mCurrentX = 0;

		for (; mCurrentX < w; ++mCurrentX) {
			if (!enoughRaySpace(1))
				break;

			const uint32 fx = mCurrentX + mTile->sx();

			Ray ray		   = mTile->constructCameraRay(fx, fy,
												   mTile->samplesRendered());
			ray.PixelIndex = fy * mTile->context()->width() + fx;

			enqueueRay(ray);
		}
	}

	/*{
		std::stringstream sstream;
		sstream << "rays_t" << mTile->index()
				<< "_p" << (mCurrentY * w + mCurrentX)
				<< "_s" << mTile->samplesRendered() << ".rdmp";
		mCoherentRayStream->dump(sstream.str());
	}*/

	return true;
}

void RenderTileSession::startShadingGroup(const ShadingGroup& grp,
										  IEntity*& entity, IMaterial*& material)
{
	entity   = getEntity(grp.EntityID);
	material = getMaterial(grp.MaterialID);
}

void RenderTileSession::endShadingGroup(const ShadingGroup& grp)
{
}

ShadowHit RenderTileSession::traceShadowRay(const Ray& ray) const
{
	mTile->statistics().addShadowRayCount();
	return mTile->context()->scene()->traceShadowRay(ray);
}

void RenderTileSession::pushFragment(const ShadingPoint& pt,
									 const LightPath& path) const
{
	mTile->context()->output()->pushFragment(pt.Ray.PixelIndex, pt, path);
}

void RenderTileSession::pushNonHitFragment(const ShadingPoint& pt) const
{
	mTile->context()->output()->pushBackgroundFragment(
		pt.Ray.PixelIndex, pt.Ray.WavelengthIndex);
}

IEntity* RenderTileSession::pickRandomLight(GeometryPoint& pt, float& pdf) const
{
	const auto& lights = mTile->context()->lights();
	if (lights.empty()) {
		pdf = 0;
		return nullptr;
	}

	size_t pick = mTile->random().get32(0, lights.size());
	pdf			= 1.0f / lights.size();

	IEntity* light = lights.at(pick).get();

	float pdf2	= 0;
	uint32 faceID = 0;
	Vector2f uv   = light->pickRandomPoint(mTile->random().get2D(), faceID, pdf2);

	light->provideGeometryPoint(faceID, uv(0), uv(1), pt);
	pdf *= pdf2;

	return light;
}
} // namespace PR
