// IWYU pragma: private, include "scene/Scene.h"
namespace PR {

// FIXME: Coherent ray tracing is still buggy
#define PR_FORCE_SINGLE_TRACE

template <typename Func>
void Scene::traceRays(RayStream& rays, HitStream& hits, Func nonHit) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	// First sort all rays
	rays.sort();

	// Split stream into specific groups
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		if (grp.isCoherent())
			traceCoherentRays(rays, grp, hits, nonHit);
		else
			traceIncoherentRays(rays, grp, hits, nonHit);
	}
}

template <typename Func, uint32 K>
inline void _sceneCheckHit(const RayGroup& grp, uint32 off, const CollisionOutput& out,
						   HitStream& hits, Func nonHit)
{
	const uint32 id = off + K;
	if (id >= grp.size()) // Ignore bad tails
		return;

	float hitD = simdpp::extract<K>(out.HitDistance);
	if (hitD > 0 && hitD < std::numeric_limits<float>::infinity()) {
		HitEntry entry;
		entry.SessionRayID = id + grp.offset();
		entry.RayID		   = grp.stream()->linearID(entry.SessionRayID);
		entry.MaterialID   = simdpp::extract<K>(out.MaterialID);
		entry.EntityID	 = simdpp::extract<K>(out.EntityID);
		entry.PrimitiveID  = simdpp::extract<K>(out.FaceID);
		for (int i = 0; i < 3; ++i)
			entry.Parameter[i] = simdpp::extract<K>(out.Parameter[i]);
		entry.Flags = simdpp::extract<K>(out.Flags);

		PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
		hits.add(entry);
	} else {
		nonHit(id + grp.offset(), grp.getRay(id));
	}
}

template <typename Func>
void Scene::traceCoherentRays(RayStream& rays, const RayGroup& grp,
							  HitStream& hits, Func nonHit) const
{
	PR_PROFILE_THIS;

#ifdef PR_FORCE_SINGLE_TRACE
	traceIncoherentRays(rays, grp, hits, nonHit);
#else
	RayPackage in;
	CollisionOutput out;

	// In some cases the group size will be not a multiply of the simd bandwith.
	// The internal stream is always a multiply therefore garbage may be traced
	// but no internal data will be corrupted.
	for (size_t i = 0;
		 i < grp.size();
		 i += PR_SIMD_BANDWIDTH) {
		in = grp.getRayPackage(i);

		// Check for collisions
		mKDTree->checkCollision(
			in, out,
			[this](const RayPackage& in2, uint64 index,
				   CollisionOutput& out2) {
				mEntities[index]->checkCollision(in2, out2);
			});

		static_assert(PR_SIMD_BANDWIDTH == 4,
					  "This implementation only works with bandwidth 4");
		_sceneCheckHit<Func, 0>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 1>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 2>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 3>(grp, i, out, hits, nonHit);
	}
#endif //PR_FORCE_SINGLE_TRACE
}

template <typename Func>
void Scene::traceIncoherentRays(RayStream& rays, const RayGroup& grp,
								HitStream& hits, Func nonHit) const
{
	PR_PROFILE_THIS;

	for (size_t i = 0;
		 i < grp.size();
		 ++i) {
		Ray in = grp.getRay(i);
		rays.invalidateRay(i + grp.offset());

		HitEntry entry;
		if (traceRay(in, entry)) {
			entry.SessionRayID = static_cast<uint32>(i + grp.offset());
			entry.RayID		   = static_cast<uint32>(grp.stream()->linearID(entry.SessionRayID));
			PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
			hits.add(entry);
		} else {
			nonHit(i + grp.offset(), in);
		}
	}
}

bool Scene::traceRay(const Ray& in, HitEntry& entry) const
{
	PR_PROFILE_THIS;

	SingleCollisionOutput out;
	mKDTree->checkCollision(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			mEntities[index]->checkCollision(in2, out2);
		});

	float hitD = out.HitDistance;
	if (hitD > 0 && hitD < std::numeric_limits<float>::infinity()) {
		entry.SessionRayID = 0;
		entry.RayID		   = 0;
		entry.MaterialID   = out.MaterialID;
		entry.EntityID	 = out.EntityID;
		entry.PrimitiveID  = out.FaceID;
		for (int i = 0; i < 3; ++i)
			entry.Parameter[i] = out.Parameter[i];
		entry.Flags = out.Flags;
		return true;
	} else {
		return false;
	}
}

ShadowHit Scene::traceShadowRay(const Ray& in) const
{
	PR_PROFILE_THIS;

	PR_ASSERT(mKDTree, "kdTree has to be valid");

	SingleCollisionOutput out;
	ShadowHit hit;

	hit.Successful = mKDTree->checkCollision(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			mEntities[index]->checkCollision(in2, out2);
		});

	for (int i = 0; i < 3; ++i)
		hit.Parameter[i] = out.Parameter[i];
	hit.EntityID	= out.EntityID;
	hit.PrimitiveID = out.FaceID;

	return hit;
}

} // namespace PR