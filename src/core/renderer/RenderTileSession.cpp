#include "RenderTileSession.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "buffer/FrameBufferBucket.h"
#include "material/IMaterial.h"
#include "output/OutputQueue.h"
#include "sampler/SplitSample.h"
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

	EntityGeometryQueryPoint query;
	query.PrimitiveID = entry.PrimitiveID;
	query.UV		  = Vector2f(entry.Parameter[0], entry.Parameter[1]);
	query.View		  = ray.Direction;
	query.Position	  = ray.t(entry.Parameter.z());

	entity->provideGeometryPoint(query, pt);

	material = getMaterial(pt.MaterialID);

	return true;
}

bool RenderTileSession::traceShadowRay(const Ray& ray, float distance, uint32 entity_id) const
{
	PR_PROFILE_THIS;

	mTile->statistics().addShadowRayCount();
	return mTile->context()->scene()->traceShadowRay(ray, distance, entity_id);
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

IEntity* RenderTileSession::pickRandomLight(const Vector3f& view, uint32 ignore_id, const Vector3f& rnd, GeometryPoint& pt, float& pdf) const
{
	PR_PROFILE_THIS;

	const auto& lights = mTile->context()->lights();
	if (lights.empty()) {
		pdf = 0;
		return nullptr;
	}

	SplitSample1D pickS(rnd(0), 0, lights.size());
	uint32 pick = pickS.integral();
	pdf			= 1.0f / lights.size();

	IEntity* light = lights.at(pick).get();
	if (light->id() == ignore_id) {
		if (lights.size() == 1) { // The light we have to ignore is the only light available
			pdf = 0;
			return nullptr;
		}

		uint32 pick2 = std::min<uint32>(lights.size() - 2, pickS.uniform() * (lights.size() - 1));
		pdf			 = 1.0f / (lights.size() - 1);

		// We use a trick to ignore the one pick we already had
		// [========P+++++++++]
		if (pick2 < pick)
			pick = pick2;
		else
			pick = pick2 + 1;

		light = lights.at(pick).get();
	}

	EntityRandomPoint rnd_p = light->pickRandomParameterPoint(view, Vector2f(rnd(1), rnd(2)));

	EntityGeometryQueryPoint query;
	query.PrimitiveID = rnd_p.PrimitiveID;
	query.UV		  = rnd_p.UV;
	query.View		  = view;
	query.Position	  = rnd_p.Position;
	light->provideGeometryPoint(query, pt);
	pdf *= rnd_p.PDF_A;

	return light;
}

constexpr size_t PR_LARGE_LIGHT_THRESHOLD = 1000;
float RenderTileSession::pickRandomLightPDF(const Vector3f& view, uint32 ignore_id, IEntity* light) const
{
	PR_UNUSED(view);
	PR_ASSERT(light->id() != ignore_id, "Picked light can not be the one ignored!");

	const auto& lights = mTile->context()->lights();
	if (lights.empty())
		return 0.0f;

	// TODO: Maybe have an approximation for large amounts of lights?
	if (lights.size() < PR_LARGE_LIGHT_THRESHOLD) {
		for (const auto& l : lights) {
			if (l->id() == ignore_id)
				return 1.0f / ((lights.size() - 1) * light->surfaceArea());
		}
	}

	return 1.0f / (lights.size() * light->surfaceArea());
}
} // namespace PR
