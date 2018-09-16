#pragma once

#include "math/SIMD.h"

namespace PR {

struct PR_LIB_INLINE CollisionInput {
	vfloat RayOrigin[3];
	vfloat RayDirection[3];
	vfloat RayInvDirection[3];

	inline void setupInverse()
	{
		for (int i = 0; i < 3; ++i)
			RayInvDirection[i] = 1 / RayDirection[i];
	}

	inline void normalize()
	{
		simdpp::float32v n = 1 / (RayDirection[0] * RayDirection[0] + RayDirection[1] * RayDirection[1] + RayDirection[2] * RayDirection[2]);

		for (int i = 0; i < 3; ++i)
			RayDirection[i] = RayDirection[i] * n;
	}

	CollisionInput transform(const Eigen::Matrix4f& oM, const Eigen::Matrix3f& dM) const
	{
		CollisionInput other;
		transformV(oM,
				   RayOrigin[0], RayOrigin[1], RayOrigin[2],
				   other.RayOrigin[0], other.RayOrigin[1], other.RayOrigin[2]);
		transformV(dM,
				   RayDirection[0], RayDirection[1], RayDirection[2],
				   other.RayDirection[0], other.RayDirection[1], other.RayDirection[2]);

		other.normalize();
		other.setupInverse();

		return other;
	}
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