#pragma once

#include "math/SIMD.h"

namespace PR {

struct PR_LIB SingleCollisionOutput {
	float HitDistance  = std::numeric_limits<float>::infinity();
	uint32 MaterialID  = 0;
	uint32 EntityID	= 0;
	uint32 FaceID	  = 0;
	Vector3f Parameter = { 0, 0, 0 };
	uint32 Flags	   = 0;
};

struct PR_LIB CollisionOutput {
	vfloat HitDistance  = vfloat(std::numeric_limits<float>::infinity());
	vuint32 MaterialID  = vuint32(0);
	vuint32 EntityID	= vuint32(0);
	vuint32 FaceID		= vuint32(0);
	Vector3fv Parameter = { vfloat(0), vfloat(0), vfloat(0) };
	vuint32 Flags		= vuint32(0);

	inline void blendFrom(const CollisionOutput& other, simdpp::mask_float32v mask)
	{
		HitDistance = blend(other.HitDistance, HitDistance, mask);
		MaterialID  = blend(other.MaterialID, MaterialID, mask);
		EntityID	= blend(other.EntityID, EntityID, mask);
		FaceID		= blend(other.FaceID, FaceID, mask);
		for (int i = 0; i < 3; ++i)
			Parameter[i] = blend(other.Parameter[i], Parameter[i], mask);
		Flags = blend(other.Flags, Flags, mask);
	}

	inline void insert(uint32 i, const SingleCollisionOutput& single)
	{
		HitDistance = PR::insert(i, HitDistance, single.HitDistance);
		MaterialID  = PR::insert(i, MaterialID, single.MaterialID);
		EntityID	= PR::insert(i, EntityID, single.EntityID);
		FaceID		= PR::insert(i, FaceID, single.FaceID);
		for (int i = 0; i < 3; ++i)
			Parameter[i] = PR::insert(i, Parameter[i], single.Parameter[i]);
		Flags = PR::insert(i, Flags, single.Flags);
	}
};
} // namespace PR