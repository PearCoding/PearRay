#include "RenderTileSession.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/OutputBuffer.h"
#include "material/IMaterial.h"
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
	PR_PROFILE_THIS;

	const uint32 w = mTile->width();
	const uint32 h = mTile->height();

	if (mCurrentY >= h)
		return false;

	mRayStream->reset();
	mHitStream->reset();

	bool forceBreak = false;
	for (; mCurrentY < h; ++mCurrentY) {
		const uint32 fy = mCurrentY + mTile->sy();

		// Allow continuation
		if (mCurrentX >= w)
			mCurrentX = 0;

		for (; mCurrentX < w; ++mCurrentX) {
			if (!enoughRaySpace(1) || mTile->context()->isStopping()) {
				forceBreak = true;
				break;
			}

			const uint32 fx = mCurrentX + mTile->sx();

			Ray ray		   = mTile->constructCameraRay(fx, fy,
												   mTile->samplesRendered());
			ray.PixelIndex = fy * mTile->context()->width() + fx;

			enqueueRay(ray);
		}

		if (forceBreak)
			break;
	}

	/*{
		std::stringstream sstream;
		sstream << "rays_t" << mTile->index()
				<< "_p" << (mCurrentY * w + mCurrentX)
				<< "_s" << mTile->samplesRendered() << ".rdmp";
		mCoherentRayStream->dump(sstream.str());
	}*/

	return !mTile->context()->isStopping();
}

void RenderTileSession::startShadingGroup(const ShadingGroup& grp,
										  IEntity*& entity, IMaterial*& material)
{
	PR_PROFILE_THIS;

	entity   = getEntity(grp.EntityID);
	material = getMaterial(grp.MaterialID);

	if (material)
		material->startGroup(grp.size(), *this);
}

void RenderTileSession::endShadingGroup(const ShadingGroup& grp)
{
	PR_PROFILE_THIS;

	IMaterial* material = getMaterial(grp.MaterialID);
	if (material)
		material->endGroup();
}

ShadowHit RenderTileSession::traceShadowRay(const Ray& ray) const
{
	PR_PROFILE_THIS;

	mTile->statistics().addShadowRayCount();
	return mTile->context()->scene()->traceShadowRay(ray);
}

void RenderTileSession::pushFragment(const ShadingPoint& pt,
									 const LightPath& path) const
{
	PR_PROFILE_THIS;

	mTile->context()->output()->pushFragment(pt.Ray.PixelIndex, pt, path);
}

void RenderTileSession::pushFeedbackFragment(const Ray& ray, uint32 feedback) const
{
	PR_PROFILE_THIS;

	mTile->context()->output()->pushFeedbackFragment(
		ray.PixelIndex, feedback);
}

IEntity* RenderTileSession::pickRandomLight(GeometryPoint& pt, float& pdf) const
{
	PR_PROFILE_THIS;

	const auto& lights = mTile->context()->lights();
	if (lights.empty()) {
		pdf = 0;
		return nullptr;
	}

	size_t pick = mTile->random().get32(0, (uint32)lights.size());
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
