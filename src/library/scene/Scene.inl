namespace PR {

template <typename Func>
void Scene::traceCoherentRays(RayStream& rays, HitStream& hits, Func nonHit) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	RayPackage in;
	CollisionOutput out;
	SingleCollisionOutput outS;

	// First sort all rays
	rays.sort();

	// Split stream into specific groups
	// TODO: Currently only one group is present
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		/*for (size_t i = 0; i < grp.Size; ++i) {
			Ray ray  = grp.getRay(i);
			mKDTree->checkCollision(ray, outS,
											   [this](const Ray& in2, uint64 index, SingleCollisionOutput& out2) {
												   mEntities[index]->checkCollision(in2, out2);
											   });

			float hitD = outS.HitDistance;
			if (hitD < std::numeric_limits<float>::infinity()) {
				HitEntry entry;
				entry.RayID		  = i;
				entry.MaterialID  = outS.MaterialID;
				entry.EntityID	= outS.EntityID;
				entry.PrimitiveID = outS.FaceID;
				entry.UV[0]		  = outS.UV[0];
				entry.UV[1]		  = outS.UV[1];
				entry.Flags		  = outS.Flags;

				PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
				hits.add(entry);
			} else {
				nonHit(ray);
			}
		}
		continue;*/

		// In some cases the group size will be not a multiply of the simd bandwith.
		// The internal stream is always a multiply therefore garbage may be traced
		// but no internal data will be corrupted.
		for (size_t i = 0;
			 i < grp.Size;
			 i += PR_SIMD_BANDWIDTH) {
			in = grp.getRayPackage(i);

			// Check for collisions
			mKDTree
				->checkCollision(in, out,
								 [this](const RayPackage& in2, uint64 index, CollisionOutput& out2) {
									 mEntities[index]->checkCollision(in2, out2);
								 });

			// TODO: Split
			for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
				if (i + k >= grp.Size) // Ignore bad tails
					break;

				float hitD = extract(k, out.HitDistance);
				if (hitD < std::numeric_limits<float>::infinity()) {
					HitEntry entry;
					entry.RayID		  = i;
					entry.MaterialID  = extract(k, out.MaterialID);
					entry.EntityID	= extract(k, out.EntityID);
					entry.PrimitiveID = extract(k, out.FaceID);
					entry.UV[0]		  = extract(k, out.UV[0]);
					entry.UV[1]		  = extract(k, out.UV[1]);
					entry.Flags		  = extract(k, out.Flags);

					PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
					hits.add(entry);
				} else {
					nonHit(grp.getRay(i + k));
				}
			}
		}
	}
}

template <typename Func>
void Scene::traceIncoherentRays(RayStream& rays, HitStream& hits, Func nonHit) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	Ray in;
	SingleCollisionOutput out;

	// First sort all rays
	rays.sort();

	// Split stream into specific groups
	// TODO: Currently only one group is present

	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		for (size_t i = 0;
			 i < grp.Size;
			 ++i) {
			for (int j = 0; j < 3; ++j)
				in.Origin[j] = grp.Origin[j][i];

			// Decompress
			from_oct(
				from_snorm16(grp.Direction[0][i]),
				from_snorm16(grp.Direction[1][i]),
				in.Direction[0], in.Direction[1], in.Direction[2]);

			in.setupInverse();

			// Check for collisions
			mKDTree
				->checkCollision(in, out,
								 [this](const Ray& in2, uint64 index, SingleCollisionOutput& out2) {
									 mEntities[index]->checkCollision(in2, out2);
								 });

			if (out.HitDistance < std::numeric_limits<float>::infinity()) { // Hit
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
}

ShadowHit Scene::traceShadowRay(const Ray& in) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	SingleCollisionOutput out;
	ShadowHit hit;

	hit.Successful = mKDTree->checkCollision(in, out,
											 [this](const Ray& in2, uint64 index, SingleCollisionOutput& out2) {
												 mEntities[index]->checkCollision(in2, out2);
											 });

	hit.EntityID	= out.EntityID;
	hit.PrimitiveID = out.FaceID;

	return hit;
}

} // namespace PR