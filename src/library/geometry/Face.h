#pragma once

#include "Triangle.h"

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

	inline Vector3f interpolateVertices(const Vector2f& local_uv) const
	{
		return V[1] * local_uv(0) + V[2] * local_uv(1) + V[0] * (1 - local_uv(0) - local_uv(1));
	}

	// Local UV!
	inline Vector3f interpolateNormals(const Vector2f& local_uv) const
	{
		return N[1] * local_uv(0) + N[2] * local_uv(1) + N[0] * (1 - local_uv(0) - local_uv(1));
	}

	inline Vector2f interpolateUVs(const Vector2f& local_uv) const
	{
		return UV[1] * local_uv(0) + UV[2] * local_uv(1) + UV[0] * (1 - local_uv(0) - local_uv(1));
	}

	inline Vector2f mapGlobalToLocalUV(const Vector2f& uv) const
	{
		Vector2f duv1 = UV[1] - UV[0];
		Vector2f duv2 = UV[2] - UV[0];

		const float det = duv2(1) * duv1(0) - duv1(1) * duv2(0);

		float nu = duv2(1) * (uv(0) - UV[0](0)) - (uv(1) - UV[0](1)) * duv2(0);
		float nv = -duv1(1) * (uv(0) - UV[0](0)) + (uv(1) - UV[0](1)) * duv1(0);

		return Vector2f(nu, nv) / det;
	}

	inline float surfaceArea() const
	{
		return Triangle::surfaceArea(V[0], V[1], V[2]);
	}

	inline void interpolate(const Vector2f& local_uv,
							Vector3f& vec, Vector3f& norm, Vector2f& uv) const
	{
		vec  = interpolateVertices(local_uv);
		norm = interpolateNormals(local_uv);
		uv   = interpolateUVs(local_uv);
	}

	// Based on http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#tangent-and-bitangent
	inline void tangentFromUV(const Vector3f& n, Vector3f& nx, Vector3f& ny) const
	{
		Vector3f dp1 = V[1] - V[0];
		Vector3f dp2 = V[2] - V[0];

		Vector2f duv1 = UV[1] - UV[0];
		Vector2f duv2 = UV[2] - UV[0];

		const float det = duv1(0) * duv2(1) - duv1(1) * duv2(0);

		nx = (dp1 * duv2(1) - dp2 * duv1(1)) / det;
		nx = nx - n * n.dot(nx);
		nx.normalize();
		ny = n.cross(nx);
	}

	uint32 MaterialSlot;
};
} // namespace PR
