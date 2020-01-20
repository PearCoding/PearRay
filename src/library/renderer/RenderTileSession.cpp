#include "RenderTileSession.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/OutputBuffer.h"
#include "buffer/OutputBufferBucket.h"
#include "material/IMaterial.h"
#include "scene/Scene.h"
#include "shader/ShadingPoint.h"

namespace PR {
RenderTileSession::RenderTileSession()
	: mThread(0)
	, mTile(nullptr)
	, mBucket()
	, mRayStream(nullptr)
	, mHitStream(nullptr)
	, mCurrentPixelPos(Point2i::Zero())
{
}

RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
									 const std::shared_ptr<OutputBufferBucket>& bucket,
									 RayStream* rayStream,
									 HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mBucket(bucket)
	, mRayStream(rayStream)
	, mHitStream(hitStream)
	, mCurrentPixelPos(Point2i::Zero())
{
	bucket->shrinkView(mTile->viewSize());
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

	const Size2i size  = mTile->viewSize();
	const uint32 slice = mTile->imageSize().Width;

	if (mCurrentPixelPos(1) >= size.Height)
		return false;

	mRayStream->reset();
	mHitStream->reset();

	bool forceBreak		   = false;
	const uint32 iterCount = mTile->iterationCount();
	for (; mCurrentPixelPos(1) < size.Height; ++mCurrentPixelPos(1)) {
		// Allow continuation
		if (mCurrentPixelPos(0) >= size.Width)
			mCurrentPixelPos(0) = 0;

		for (; mCurrentPixelPos(0) < size.Width; ++mCurrentPixelPos(0)) {
			if (!enoughRaySpace(1) || mTile->context()->isStopping()) {
				forceBreak = true;
				break;
			}

			const Point2i p = mCurrentPixelPos + mTile->start();

			Ray ray		   = mTile->constructCameraRay(p, iterCount);
			ray.PixelIndex = p(1) * slice + p(0);

			enqueueCameraRay(ray);
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

bool RenderTileSession::traceBounceRay(const Ray& ray, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const
{
	PR_PROFILE_THIS;
	mTile->statistics().addBounceRayCount();

	HitEntry entry;
	if (!mTile->context()->scene()->traceSingleRay(ray, entry))
		return false;

	entity = getEntity(entry.EntityID);
	if (!entity)
		return false;

	material = getMaterial(entry.MaterialID);

	entity->provideGeometryPoint(ray.Direction, entry.PrimitiveID,
								 Vector3f(entry.Parameter[0], entry.Parameter[1], entry.Parameter[2]), pt);

	return true;
}

ShadowHit RenderTileSession::traceShadowRay(const Ray& ray) const
{
	PR_PROFILE_THIS;

	mTile->statistics().addShadowRayCount();
	return mTile->context()->scene()->traceShadowRay(ray);
}

Point2i RenderTileSession::localCoordinates(Point1i pixelIndex) const
{
	const Size1i slice = mTile->context()->viewSize().Width;
	Point1i gpx		   = pixelIndex % slice;
	Point1i gpy		   = pixelIndex / slice;

	PR_ASSERT(gpx >= mTile->start().x() && gpx < mTile->end().x(),
			  "Invalid fragment push operation");
	PR_ASSERT(gpy >= mTile->start().y() && gpy < mTile->end().y(),
			  "Invalid fragment push operation");

	return Point2i(gpx, gpy) - mTile->start();
}

void RenderTileSession::pushSpectralFragment(const ColorTriplet& spec, const Ray& ray,
											 const LightPath& path) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(ray.PixelIndex);
	mBucket->pushSpectralFragment(coords, spec * ray.Weight,
								  ray.WavelengthIndex, ray.Flags & RF_Monochrome, path);
}

void RenderTileSession::pushSPFragment(const ShadingPoint& pt,
									   const LightPath& path) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(pt.Ray.PixelIndex);
	mBucket->pushSPFragment(coords, pt, path);
}

void RenderTileSession::pushFeedbackFragment(uint32 feedback, const Ray& ray) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(ray.PixelIndex);
	mBucket->pushFeedbackFragment(coords, feedback);
}

IEntity* RenderTileSession::pickRandomLight(const Vector3f& view, GeometryPoint& pt, float& pdf) const
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

	float pdf2	 = 0;
	uint32 faceID  = 0;
	Vector3f param = light->pickRandomParameterPoint(view, mTile->random().get2D(), faceID, pdf2);

	light->provideGeometryPoint(view, faceID, param, pt);
	pdf *= pdf2;

	return light;
}
} // namespace PR
