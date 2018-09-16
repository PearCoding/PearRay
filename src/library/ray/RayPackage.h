#pragma once

#include "math/SIMD.h"

namespace PR {

struct PR_LIB_INLINE RayPackage {
	vfloat Origin[3];
	vfloat Direction[3];
	vfloat InvDirection[3];

	inline void setupInverse()
	{
		for (int i = 0; i < 3; ++i)
			InvDirection[i] = 1 / Direction[i];
	}

	inline void normalize()
	{
		vfloat n = 1 / (Direction[0] * Direction[0] + Direction[1] * Direction[1] + Direction[2] * Direction[2]);

		for (int i = 0; i < 3; ++i)
			Direction[i] = Direction[i] * n;
	}

	RayPackage transform(const Eigen::Matrix4f& oM, const Eigen::Matrix3f& dM) const
	{
		RayPackage other;
		transformV(oM,
				   Origin[0], Origin[1], Origin[2],
				   other.Origin[0], other.Origin[1], other.Origin[2]);
		transformV(dM,
				   Direction[0], Direction[1], Direction[2],
				   other.Direction[0], other.Direction[1], other.Direction[2]);

		other.normalize();
		other.setupInverse();

		return other;
	}
};

} // namespace PR