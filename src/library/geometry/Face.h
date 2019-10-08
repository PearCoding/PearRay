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

	// Based on http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#tangent-and-bitangent
	inline void tangentFromUV(Vector3f& nx, Vector3f& ny) const
	{
		Vector3f dp1 = V[1] - V[0];
		Vector3f dp2 = V[2] - V[0];

		Vector2f duv1 = UV[1] - UV[0];
		Vector2f duv2 = UV[2] - UV[0];

		float r = 1.0f / (duv1(0) * duv2(1) - duv1(1) * duv2(0));

		nx = (dp1 * duv2(1) - dp2 * duv1(1)) * r;
		ny = (dp2 * duv1(0) - dp1 * duv2(0)) * r;
	}

	uint32 MaterialSlot;
};
} // namespace PR
