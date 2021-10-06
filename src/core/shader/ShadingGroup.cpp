#include "ShadingGroup.h"
#include "Profiler.h"
#include "material/IMaterial.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"

namespace PR {
ShadingGroup::ShadingGroup(const ShadingGroupBlock& blck, const StreamPipeline* pipeline, const RenderTileSession& session)
	: mBlock(blck)
	, mPipeline(pipeline)
	, mEntity(nullptr)
{
	PR_PROFILE_THIS;

	mEntity = session.getEntity(blck.EntityID);
}

ShadingGroup::~ShadingGroup()
{
	PR_PROFILE_THIS;
}

void ShadingGroup::extractHitEntry(size_t i, HitEntry& entry) const
{
	entry = mBlock.Stream->get(mBlock.Start + i);
}

void ShadingGroup::extractRay(size_t i, Ray& ray) const
{
	auto rayID = mBlock.Stream->get(mBlock.Start + i).RayID;
	ray		   = mPipeline->getTracedRay(rayID);
}

void ShadingGroup::extractRayGroup(size_t i, RayGroup& rayGroup) const
{
	Ray ray;
	extractRay(i, ray);
	rayGroup = mPipeline->getRayGroup(ray.GroupID);
}

void ShadingGroup::extractGeometryPoint(size_t i, GeometryPoint& pt) const
{
	auto entry = mBlock.Stream->get(mBlock.Start + i);
	auto ray   = mPipeline->getTracedRay(entry.RayID);

	EntityGeometryQueryPoint query;
	query.View		  = ray.Direction;
	query.Position	  = ray.t(entry.Parameter.z());
	query.PrimitiveID = entry.PrimitiveID;
	query.UV		  = Vector2f(entry.Parameter[0], entry.Parameter[1]);
	mEntity->provideGeometryPoint(query, pt);
	pt.EntityID = mBlock.EntityID;
}

void ShadingGroup::computeShadingPoint(size_t i, IntersectionPoint& spt) const
{
	auto entry = mBlock.Stream->get(mBlock.Start + i);

	spt.Ray = mPipeline->getTracedRay(entry.RayID);

	EntityGeometryQueryPoint query;
	query.View		  = spt.Ray.Direction;
	query.Position	  = spt.Ray.t(entry.Parameter.z());
	query.PrimitiveID = entry.PrimitiveID;
	query.UV		  = Vector2f(entry.Parameter[0], entry.Parameter[1]);
	mEntity->provideGeometryPoint(query, spt.Surface.Geometry);
	spt.Surface.Geometry.EntityID = mBlock.EntityID;

	spt.setForSurface(spt.Ray, query.Position, spt.Surface.Geometry);
}
} // namespace PR
