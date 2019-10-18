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
	// TODO: Currently only one group is present
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		if (grp.Coherent)
			traceCoherentRays(grp, hits, nonHit);
		else
			traceIncoherentRays(grp, hits, nonHit);
	}
}

template <typename Func, uint32 K>
inline void _sceneCheckHit(const RayGroup& grp, uint32 off, const CollisionOutput& out,
						   HitStream& hits, Func nonHit)
{
	const uint32 id = off + K;
	if (id >= grp.Size) // Ignore bad tails
		return;

	float hitD = simdpp::extract<K>(out.HitDistance);
	if (hitD > 0 && hitD < std::numeric_limits<float>::infinity()) {
		HitEntry entry;
		entry.RayID		  = id;
		entry.MaterialID  = simdpp::extract<K>(out.MaterialID);
		entry.EntityID	= simdpp::extract<K>(out.EntityID);
		entry.PrimitiveID = simdpp::extract<K>(out.FaceID);
		entry.UV[0]		  = simdpp::extract<K>(out.UV[0]);
		entry.UV[1]		  = simdpp::extract<K>(out.UV[1]);
		entry.Flags		  = simdpp::extract<K>(out.Flags);

		PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
		hits.add(entry);
	} else {
		nonHit(grp.getRay(id));
	}
}

template <typename Func>
void Scene::traceCoherentRays(const RayGroup& grp, HitStream& hits, Func nonHit) const
{
#ifdef PR_FORCE_SINGLE_TRACE
	traceIncoherentRays(grp, hits, nonHit);
#else
	RayPackage in;
	CollisionOutput out;

	// In some cases the group size will be not a multiply of the simd bandwith.
	// The internal stream is always a multiply therefore garbage may be traced
	// but no internal data will be corrupted.
	for (size_t i = 0;
		 i < grp.Size;
		 i += PR_SIMD_BANDWIDTH) {
		in = grp.getRayPackage(i);

		// Check for collisions
		mKDTree->checkCollision(
			in, out,
			[this](const RayPackage& in2, uint64 index,
				   CollisionOutput& out2) {
				mEntities[index]->checkCollision(in2, out2);
			});

		static_assert(PR_SIMD_BANDWIDTH == 4, "This implementation onyl works with bandwidth 4");
		_sceneCheckHit<Func, 0>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 1>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 2>(grp, i, out, hits, nonHit);
		_sceneCheckHit<Func, 3>(grp, i, out, hits, nonHit);
	}
#endif //PR_FORCE_SINGLE_TRACE
}

template <typename Func>
void Scene::traceIncoherentRays(const RayGroup& grp, HitStream& hits, Func nonHit) const
{
	Ray in;
	SingleCollisionOutput out;

	for (size_t i = 0;
		 i < grp.Size;
		 ++i) {
		in = grp.getRay(i);

		// Check for collisions
		mKDTree->checkCollision(
			in, out,
			[this](const Ray& in2, uint64 index,
				   SingleCollisionOutput& out2) {
				mEntities[index]->checkCollision(in2, out2);
			});

		float hitD = out.HitDistance;
		if (hitD > 0 && hitD < std::numeric_limits<float>::infinity()) {
			HitEntry entry;
			entry.RayID		  = i;
			entry.MaterialID  = out.MaterialID;
			entry.EntityID	= out.EntityID;
			entry.PrimitiveID = out.FaceID;
			entry.UV[0]		  = out.UV[0];
			entry.UV[1]		  = out.UV[1];
			entry.Flags		  = out.Flags;

			PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
			hits.add(entry);
		} else {
			nonHit(in);
		}
	}
}

ShadowHit Scene::traceShadowRay(const Ray& in) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	SingleCollisionOutput out;
	ShadowHit hit;

	hit.Successful = mKDTree->checkCollision(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			mEntities[index]->checkCollision(in2, out2);
		});

	hit.EntityID	= out.EntityID;
	hit.PrimitiveID = out.FaceID;

	return hit;
}

} // namespace PR