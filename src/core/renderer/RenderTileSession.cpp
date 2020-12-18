#include "RenderTileSession.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "StreamPipeline.h"
#include "buffer/FrameBufferBucket.h"
#include "material/IMaterial.h"
#include "math/SplitSample.h"
#include "output/OutputQueue.h"
#include "scene/Scene.h"
#include "scene/SceneDatabase.h"
#include "trace/IntersectionPoint.h"

namespace PR {
RenderTileSession::RenderTileSession()
	: mThread(0)
	, mTile(nullptr)
	, mPipeline(nullptr)
	, mOutputQueue()
{
}

RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile, StreamPipeline* pipeline,
									 const std::shared_ptr<OutputQueue>& queue,
									 const std::shared_ptr<FrameBufferBucket>& bucket)
	: mThread(thread)
	, mTile(tile)
	, mPipeline(pipeline)
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
	return context()->scene()->database()->Entities->getSafe(id).get();
}

IMaterial* RenderTileSession::getMaterial(uint32 id) const
{
	return context()->scene()->database()->Materials->getSafe(id).get();
}

IEmission* RenderTileSession::getEmission(uint32 id) const
{
	return context()->scene()->database()->Emissions->getSafe(id).get();
}

const RayGroup& RenderTileSession::getRayGroup(uint32 id) const
{
	PR_ASSERT(mPipeline, "Expected valid stream pipeline");
	return mPipeline->getRayGroup(id);
}

bool RenderTileSession::traceSingleRay(const Ray& ray, Vector3f& pos, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const
{
	PR_PROFILE_THIS;
	if (ray.Flags & RF_Camera)
		mTile->statistics().addCameraRayCount();
	else if (ray.Flags & RF_Light)
		mTile->statistics().addLightRayCount();
	else
		mTile->statistics().addBounceRayCount();

	HitEntry entry;
	if (!mTile->context()->scene()->traceSingleRay(ray, entry))
		return false;

	entity = getEntity(entry.EntityID);
	if (!entity)
		return false;

	EntityGeometryQueryPoint query;
	query.PrimitiveID = entry.PrimitiveID;
	query.UV		  = Vector2f(entry.Parameter[0], entry.Parameter[1]);
	query.View		  = ray.Direction;
	query.Position	  = ray.t(entry.Parameter.z());

	entity->provideGeometryPoint(query, pt);
	pt.EntityID = entry.EntityID;

	pos		 = query.Position;
	material = getMaterial(pt.MaterialID);

	return true;
}

bool RenderTileSession::traceShadowRay(const Ray& ray, float distance) const
{
	PR_PROFILE_THIS;

	mTile->statistics().addShadowRayCount();
	return mTile->context()->scene()->traceShadowRay(ray, distance);
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

void RenderTileSession::pushSpectralFragment(float mis, const SpectralBlob& importance, const SpectralBlob& radiance,
											 const Ray& ray, const LightPath& path) const
{
	PR_PROFILE_THIS;
	auto coords		= localCoordinates(ray.PixelIndex);
	const auto& grp = getRayGroup(ray);
	mOutputQueue->pushSpectralFragment(coords, mis, grp.Importance * importance / grp.WavelengthPDF, radiance,
									   ray.WavelengthNM, ray.Flags & RF_Monochrome, ray.GroupID, path);
	if (mOutputQueue->isReadyToCommit())
		mOutputQueue->commitAndFlush(mBucket.get());
}

void RenderTileSession::pushSPFragment(const IntersectionPoint& pt,
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
} // namespace PR
