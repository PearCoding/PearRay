#include "RenderTileSession.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/FrameBufferBucket.h"
#include "material/IMaterial.h"
#include "output/OutputQueue.h"
#include "scene/Scene.h"
#include "shader/ShadingPoint.h"

namespace PR {
RenderTileSession::RenderTileSession()
	: mThread(0)
	, mTile(nullptr)
	, mOutputQueue()
{
}

RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
									 const std::shared_ptr<OutputQueue>& queue,
									 const std::shared_ptr<FrameBufferBucket>& bucket)
	: mThread(thread)
	, mTile(tile)
	, mOutputQueue(queue)
	, mBucket(bucket)
{
	bucket->shrinkView(tile->viewSize());
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

bool RenderTileSession::traceOcclusionRay(const Ray& ray) const
{
	PR_PROFILE_THIS;

	mTile->statistics().addShadowRayCount(); //Maybe his own category?
	return mTile->context()->scene()->traceOcclusionRay(ray);
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

void RenderTileSession::pushSpectralFragment(const SpectralBlob& spec, const Ray& ray,
											 const LightPath& path) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(ray.PixelIndex);
	mOutputQueue->pushSpectralFragment(coords, spec * ray.Weight,
									   ray.WavelengthNM, ray.Flags & RF_Monochrome, path);
	if (mOutputQueue->isReadyToCommit())
		mOutputQueue->commitAndFlush(mBucket.get());
}

void RenderTileSession::pushSPFragment(const ShadingPoint& pt,
									   const LightPath& path) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(pt.Ray.PixelIndex);
	mOutputQueue->pushSPFragment(coords, pt, path);
	if (mOutputQueue->isReadyToCommit())
		mOutputQueue->commitAndFlush(mBucket.get());
}

void RenderTileSession::pushFeedbackFragment(uint32 feedback, const Ray& ray) const
{
	PR_PROFILE_THIS;
	auto coords = localCoordinates(ray.PixelIndex);
	mOutputQueue->pushFeedbackFragment(coords, feedback);
	if (mOutputQueue->isReadyToCommit())
		mOutputQueue->commitAndFlush(mBucket.get());
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

	float pdf2	   = 0;
	uint32 faceID  = 0;
	Vector3f param = light->pickRandomParameterPoint(view, mTile->random().get2D(), faceID, pdf2);

	light->provideGeometryPoint(view, faceID, param, pt);
	pdf *= pdf2;

	return light;
}
} // namespace PR
