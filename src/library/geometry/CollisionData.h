#pragma once

#include "math/SIMD.h"

namespace PR {

struct PR_LIB SingleCollisionOutput {
	float HitDistance = std::numeric_limits<float>::infinity();
	uint32 MaterialID = 0;
	uint32 EntityID   = 0;
	uint32 FaceID	 = 0;
	float UV[2]		  = { 0, 0 };
	uint32 Flags	  = 0;
};

struct PR_LIB CollisionOutput {
	vfloat HitDistance = vfloat(std::numeric_limits<float>::infinity());
	vuint32 MaterialID = vuint32(0);
	vuint32 EntityID   = vuint32(0);
	vuint32 FaceID	 = vuint32(0);
	vfloat UV[2]	   = { vfloat(0), vfloat(0) };
	vuint32 Flags	  = vuint32(0);

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