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
	, mMaterial(nullptr)
{
	PR_PROFILE_THIS;

	mEntity	  = session.getEntity(blck.EntityID);
	mMaterial = session.getMaterial(blck.MaterialID);

	if (mMaterial)
		mMaterial->startGroup(blck.size(), session);
}

ShadingGroup::~ShadingGroup()
{
	PR_PROFILE_THIS;

	if (mMaterial)
		mMaterial->endGroup();
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

void ShadingGroup::extractGeometryPoint(size_t i, GeometryPoint& pt) const
{
	auto entry = mBlock.Stream->get(mBlock.Start + i);
	auto ray   = mPipeline->getTracedRay(entry.RayID);
	mEntity->provideGeometryPoint(ray.Direction, entry.PrimitiveID, entry.Parameter, pt);
}

void ShadingGroup::computeShadingPoint(size_t i, ShadingPoint& spt) const
{
	auto entry = mBlock.Stream->get(mBlock.Start + i);

	spt.Ray = mPipeline->getTracedRay(entry.RayID);
	mEntity->provideGeometryPoint(spt.Ray.Direction, entry.PrimitiveID, entry.Parameter, spt.Geometry);
	spt.setByIdentity(spt.Ray, spt.Geometry);
}
} // namespace PR
