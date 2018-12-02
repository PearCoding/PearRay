#pragma once

#include "math/SIMD.h"

namespace PR {

struct PR_LIB_INLINE SingleCollisionOutput {
	float HitDistance;
	uint32 MaterialID;
	uint32 EntityID;
	uint32 FaceID;
	float UV[2];
	uint32 Flags;
};

struct PR_LIB_INLINE CollisionOutput {
	vfloat HitDistance;
	vuint32 MaterialID;
	vuint32 EntityID;
	vuint32 FaceID;
	vfloat UV[2];
	vuint32 Flags;

	inline void blendFrom(const CollisionOutput& other, simdpp::mask_float32v mask)
	{
		HitDistance = blend(other.HitDistance, HitDistance, mask);
		MaterialID  = blend(other.MaterialID, MaterialID, mask);
		EntityID	= blend(other.EntityID, EntityID, mask);
		FaceID		= blend(other.FaceID, FaceID, mask);
		UV[0]		= blend(other.UV[0], UV[0], mask);
		UV[1]		= blend(other.UV[1], UV[1], mask);
		Flags		= blend(other.Flags, Flags, mask);
	}
};
} // namespace PR