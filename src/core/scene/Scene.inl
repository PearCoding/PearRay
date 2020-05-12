// IWYU pragma: private, include "scene/Scene.h"
namespace PR {

bool Scene::traceSingleRay(const Ray& in, HitEntry& entry) const
{
	PR_PROFILE_THIS;

	SingleCollisionOutput out;
	mKDTree->checkCollisionSingle(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			const IEntity* entity = mEntities[index].get();
			if (entity->visibilityFlags() & in2.Flags)
				entity->checkCollision(in2, out2);
			else
				out2.Successful = false;
		});

	if (out.Successful) {
		entry.RayID		  = 0;
		entry.MaterialID  = out.MaterialID;
		entry.EntityID	  = out.EntityID;
		entry.PrimitiveID = out.FaceID;
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

	mKDTree->checkCollisionSingle(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			const IEntity* entity = mEntities[index].get();
			if (entity->visibilityFlags() & in2.Flags)
				entity->checkCollision(in2, out2);
			else
				out2.Successful = false;
		});

	hit.Successful = out.Successful;
	for (int i = 0; i < 3; ++i)
		hit.Parameter[i] = out.Parameter[i];
	hit.EntityID	= out.EntityID;
	hit.PrimitiveID = out.FaceID;

	return hit;
}

bool Scene::traceOcclusionRay(const Ray& in) const
{
	PR_PROFILE_THIS;

	PR_ASSERT(mKDTree, "kdTree has to be valid");

	SingleCollisionOutput out;

	mKDTree->checkCollisionSingle(
		in, out,
		[this](const Ray& in2, uint64 index,
			   SingleCollisionOutput& out2) {
			const IEntity* entity = mEntities[index].get();
			if (entity->visibilityFlags() & in2.Flags)
				entity->checkCollision(in2, out2);
			else
				out2.Successful = false;
		},
		true);
	return out.Successful;
}

} // namespace PR