#pragma once

#include "PR_Config.h"

namespace PR {
class Material;
class PR_LIB Face {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Face()
		: MaterialSlot(0)
	{
	}

	Vector3f V[3];
	Vector3f N[3];
	Vector2f UV[3];

	inline void interpolate(float u, float v,
							Vector3f& vec, Vector3f& norm, Vector2f& uv) const
	{
		vec = V[1] * u + V[2] * v + V[0] * (1 - u - v);

		norm = N[1] * u + N[2] * v + N[0] * (1 - u - v);
		//norm.normalize();

		uv = UV[1] * u + UV[2] * v + UV[0] * (1 - u - v);
	}

	uint32 MaterialSlot;
};
} // namespace PR
