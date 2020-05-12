#pragma once

#include "ray/RayPackage.h"

namespace PR {
namespace TriangleIntersection {
/* Doug Baldwin and Michael Weber, Fast Ray-Triangle Intersections by Coordinate Transformation,
 * Journal of Computer Graphics Techniques (JCGT), vol. 5, no. 3, 39-49, 2016
 * http://jcgt.org/published/0005/03/03/
 */
inline void constructBW12Matrix(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
								float* matrix)
{
	PR_ASSERT(matrix, "Expected valid matrix");

	Vector3f edge1	= p1 - p0;
	Vector3f edge2	= p2 - p0;
	Vector3f normal = edge1.cross(edge2);

	// Build matrix from global to barycentric coordinates.
	float x1, x2;
	float num = p0.dot(normal);

	if (fabs(normal[0]) > fabs(normal[1]) && fabs(normal[0]) > fabs(normal[2])) {
		x1 = p1[1] * p0[2] - p1[2] * p0[1];
		x2 = p2[1] * p0[2] - p2[2] * p0[1];

		matrix[0] = 0.0f;
		matrix[1] = edge2[2] / normal[0];
		matrix[2] = -edge2[1] / normal[0];
		matrix[3] = x2 / normal[0];

		matrix[4] = 0.0f;
		matrix[5] = -edge1[2] / normal[0];
		matrix[6] = edge1[1] / normal[0];
		matrix[7] = -x1 / normal[0];

		matrix[8]  = 1.0f;
		matrix[9]  = normal[1] / normal[0];
		matrix[10] = normal[2] / normal[0];
		matrix[11] = -num / normal[0];
	} else if (fabs(normal[1]) > fabs(normal[2])) {
		x1 = p1[2] * p0[0] - p1[0] * p0[2];
		x2 = p2[2] * p0[0] - p2[0] * p0[2];

		matrix[0] = -edge2[2] / normal[1];
		matrix[1] = 0.0f;
		matrix[2] = edge2[0] / normal[1];
		matrix[3] = x2 / normal[1];

		matrix[4] = edge1[2] / normal[1];
		matrix[5] = 0.0f;
		matrix[6] = -edge1[0] / normal[1];
		matrix[7] = -x1 / normal[1];

		matrix[8]  = normal[0] / normal[1];
		matrix[9]  = 1.0f;
		matrix[10] = normal[2] / normal[1];
		matrix[11] = -num / normal[1];
	} else {
		x1 = p1[0] * p0[1] - p1[1] * p0[0];
		x2 = p2[0] * p0[1] - p2[1] * p0[0];

		matrix[0] = edge2[1] / normal[2];
		matrix[1] = -edge2[0] / normal[2];
		matrix[2] = 0.0f;
		matrix[3] = x2 / normal[2];

		matrix[4] = -edge1[1] / normal[2];
		matrix[5] = edge1[0] / normal[2];
		matrix[6] = 0.0f;
		matrix[7] = -x1 / normal[2];

		matrix[8]  = normal[0] / normal[2];
		matrix[9]  = normal[1] / normal[2];
		matrix[10] = 1.0f;
		matrix[11] = -num / normal[2];
	}
}

namespace details {
template <bool Side>
inline PR_LIB_CORE bool _intersectBaseBW12(
	const Ray& in,
	const float* M,
	float& t, Vector2f& uv)
{
	const float transS = M[8] * in.Origin[0] + M[9] * in.Origin[1] + M[10] * in.Origin[2] + M[11];
	const float transD = M[8] * in.Direction[0] + M[9] * in.Direction[1] + M[10] * in.Direction[2];
	t				   = -transS / transD;

	if ((Side && !in.isInsideRange(t)) || abs(transD) <= PR_EPSILON)
		return false;

	const Vector3f wr = in.t(t);

	uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2] * wr[2] + M[3];
	uv[1] = M[4] * wr[0] + M[5] * wr[1] + M[6] * wr[2] + M[7];

	return (uv[0] >= 0.0f && uv[1] >= 0.0f && uv[0] + uv[1] <= 1.0f);
}

template <bool Side>
inline PR_LIB_CORE bfloat _intersectBaseBW12(
	const RayPackage& in,
	const float* M,
	vfloat& t, Vector2fv& uv)
{
	const vfloat transS = M[8] * in.Origin[0] + M[9] * in.Origin[1] + M[10] * in.Origin[2] + M[11];
	const vfloat transD = M[8] * in.Direction[0] + M[9] * in.Direction[1] + M[10] * in.Direction[2];
	t					= -transS / transD;

	bfloat valid = (abs(transD) > PR_EPSILON);
	if (Side)
		valid = valid & in.isInsideRange(t);
	if (none(valid))
		return valid;

	const Vector3fv wr = in.t(t);

	uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2] * wr[2] + M[3];
	uv[1] = M[4] * wr[0] + M[5] * wr[1] + M[6] * wr[2] + M[7];

	return valid & (uv[0] >= 0.0f) & (uv[1] >= 0.0f) & (uv[0] + uv[1] <= 1.0f);
}
} // namespace details
inline PR_LIB_CORE bool intersectLineBW12(
	const Ray& in,
	const float* M)
{
	float t;
	Vector2f uv;
	return details::_intersectBaseBW12<false>(in, M, t, uv);
}

inline PR_LIB_CORE bool intersectBW12(
	const Ray& in,
	const float* M,
	float& t)
{
	Vector2f uv;
	return details::_intersectBaseBW12<true>(in, M, t, uv);
}

inline PR_LIB_CORE bool intersectBW12(
	const Ray& in,
	const float* M,
	float& t, Vector2f& uv)
{
	return details::_intersectBaseBW12<true>(in, M, t, uv);
}

inline PR_LIB_CORE bfloat intersectLineBW12(
	const RayPackage& in,
	const float* M)
{
	vfloat t;
	Vector2fv uv;
	return details::_intersectBaseBW12<false>(in, M, t, uv);
}

inline PR_LIB_CORE bfloat intersectBW12(
	const RayPackage& in,
	const float* M,
	vfloat& t)
{
	Vector2fv uv;
	return details::_intersectBaseBW12<true>(in, M, t, uv);
}

inline PR_LIB_CORE bfloat intersectBW12(
	const RayPackage& in,
	const float* M,
	vfloat& t, Vector2fv& uv)
{
	return details::_intersectBaseBW12<true>(in, M, t, uv);
}
} // namespace TriangleIntersection
} // namespace PR
