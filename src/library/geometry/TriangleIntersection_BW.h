#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_BW_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
/* Doug Baldwin and Michael Weber, Fast Ray-Triangle Intersections by Coordinate Transformation,
 * Journal of Computer Graphics Techniques (JCGT), vol. 5, no. 3, 39-49, 2016
 * http://jcgt.org/published/0005/03/03/
 */
inline void constructBW9Matrix(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
							   float* matrix, int& fixedColumn)
{
	fixedColumn = -1;
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

		//Do matrix set up here for when a = 1, b = c = 0 formula
		fixedColumn = 0;

		matrix[0] = edge2[2] / normal[0];
		matrix[1] = -edge2[1] / normal[0];
		matrix[2] = x2 / normal[0];

		matrix[3] = -edge1[2] / normal[0];
		matrix[4] = edge1[1] / normal[0];
		matrix[5] = -x1 / normal[0];

		matrix[6] = normal[1] / normal[0];
		matrix[7] = normal[2] / normal[0];
		matrix[8] = -num / normal[0];
	} else if (fabs(normal[1]) > fabs(normal[2])) {
		x1 = p1[2] * p0[0] - p1[0] * p0[2];
		x2 = p2[2] * p0[0] - p2[0] * p0[2];

		// b = 1 case
		fixedColumn = 1;

		matrix[0] = -edge2[2] / normal[1];
		matrix[1] = edge2[0] / normal[1];
		matrix[2] = x2 / normal[1];

		matrix[3] = edge1[2] / normal[1];
		matrix[4] = -edge1[0] / normal[1];
		matrix[5] = -x1 / normal[1];

		matrix[6] = normal[0] / normal[1];
		matrix[7] = normal[2] / normal[1];
		matrix[8] = -num / normal[1];
	} else {
		x1 = p1[0] * p0[1] - p1[1] * p0[0];
		x2 = p2[0] * p0[1] - p2[1] * p0[0];

		// c = 1 case
		fixedColumn = 2;

		matrix[0] = edge2[1] / normal[2];
		matrix[1] = -edge2[0] / normal[2];
		matrix[2] = x2 / normal[2];

		matrix[3] = -edge1[1] / normal[2];
		matrix[4] = edge1[0] / normal[2];
		matrix[5] = -x1 / normal[2];

		matrix[6] = normal[0] / normal[2];
		matrix[7] = normal[1] / normal[2];
		matrix[8] = -num / normal[2];
	}
}

inline PR_LIB bool intersectBW9(
	const Ray& in,
	const float* M,
	int fixedColumn,
	Vector2f& uv,
	float& t)
{
	if (fixedColumn == 0) {
		const float transS = in.Origin[0] + M[6] * in.Origin[1] + M[7] * in.Origin[2] + M[8];
		const float transD = in.Direction[0] + M[6] * in.Direction[1] + M[7] * in.Direction[2];
		t				   = -transS / transD;

		if (t <= PR_TRIANGLE_BW_INTERSECT_EPSILON || abs(transD) <= PR_EPSILON)
			return false;

		const Vector3f wr = in.t(t);

		uv[0] = M[0] * wr[1] + M[1] * wr[2] + M[2];
		uv[1] = M[3] * wr[1] + M[4] * wr[2] + M[5];
	} else if (fixedColumn == 1) {
		const float transS = M[6] * in.Origin[0] + in.Origin[1] + M[7] * in.Origin[2] + M[8];
		const float transD = M[6] * in.Direction[0] + in.Direction[1] + M[7] * in.Direction[2];
		t				   = -transS / transD;

		if (t <= PR_TRIANGLE_BW_INTERSECT_EPSILON || abs(transD) <= PR_EPSILON)
			return false;

		const Vector3f wr = in.t(t);

		uv[0] = M[0] * wr[0] + M[1] * wr[2] + M[2];
		uv[1] = M[3] * wr[0] + M[4] * wr[2] + M[5];
	} else if (fixedColumn == 2) {
		const float transS = M[6] * in.Origin[0] + M[7] * in.Origin[1] + in.Origin[2] + M[8];
		const float transD = M[6] * in.Direction[0] + M[7] * in.Direction[1] + in.Direction[2];
		t				   = -transS / transD;

		if (t <= PR_TRIANGLE_BW_INTERSECT_EPSILON || abs(transD) <= PR_EPSILON)
			return false;

		const Vector3f wr = in.t(t);

		uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2];
		uv[1] = M[3] * wr[0] + M[4] * wr[1] + M[5];
	} else {
		PR_ASSERT(false, "Invalid fixed-column code");
		return false;
	}

	return (uv[0] >= 0.0f && uv[1] >= 0.0f && uv[0] + uv[1] <= 1.0f);
}

inline PR_LIB bfloat intersectBW9(
	const RayPackage& in,
	const float* M,
	int fixedColumn,
	Vector2fv& uv,
	vfloat& t)
{
	bfloat valid;

	if (fixedColumn == 0) {
		const vfloat transS = in.Origin[0] + M[6] * in.Origin[1] + M[7] * in.Origin[2] + M[8];
		const vfloat transD = in.Direction[0] + M[6] * in.Direction[1] + M[7] * in.Direction[2];
		t					= -transS / transD;

		valid = (t > PR_TRIANGLE_BW_INTERSECT_EPSILON) & (abs(transD) > PR_EPSILON);
		if (none(valid))
			return valid;

		const Vector3fv wr = in.t(t);

		uv[0] = M[0] * wr[1] + M[1] * wr[2] + M[2];
		uv[1] = M[3] * wr[1] + M[4] * wr[2] + M[5];
	} else if (fixedColumn == 1) {
		const vfloat transS = M[6] * in.Origin[0] + in.Origin[1] + M[7] * in.Origin[2] + M[8];
		const vfloat transD = M[6] * in.Direction[0] + in.Direction[1] + M[7] * in.Direction[2];
		t					= -transS / transD;

		valid = (t > PR_TRIANGLE_BW_INTERSECT_EPSILON) & (abs(transD) > PR_EPSILON);
		if (none(valid))
			return valid;

		const Vector3fv wr = in.t(t);

		uv[0] = M[0] * wr[0] + M[1] * wr[2] + M[2];
		uv[1] = M[3] * wr[0] + M[4] * wr[2] + M[5];
	} else if (fixedColumn == 2) {
		const vfloat transS = M[6] * in.Origin[0] + M[7] * in.Origin[1] + in.Origin[2] + M[8];
		const vfloat transD = M[6] * in.Direction[0] + M[7] * in.Direction[1] + in.Direction[2];
		t					= -transS / transD;

		valid = (t > PR_TRIANGLE_BW_INTERSECT_EPSILON) & (abs(transD) > PR_EPSILON);
		if (none(valid))
			return valid;

		const Vector3fv wr = in.t(t);

		uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2];
		uv[1] = M[3] * wr[0] + M[4] * wr[1] + M[5];
	} else {
		PR_ASSERT(false, "Invalid fixed-column code");
	}

	return valid & (uv[0] >= 0.0f) & (uv[1] >= 0.0f) & (uv[0] + uv[1] <= 1.0f);
}

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

inline PR_LIB bool intersectBW12(
	const Ray& in,
	const float* M,
	Vector2f& uv,
	float& t)
{
	const float transS = M[8] * in.Origin[0] + M[9] * in.Origin[1] + M[10] * in.Origin[2] + M[11];
	const float transD = M[8] * in.Direction[0] + M[9] * in.Direction[1] + M[10] * in.Direction[2];
	t				   = -transS / transD;

	if (t <= PR_TRIANGLE_BW_INTERSECT_EPSILON || abs(transD) <= PR_EPSILON)
		return false;

	const Vector3f wr = in.t(t);

	uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2] * wr[2] + M[3];
	uv[1] = M[4] * wr[0] + M[5] * wr[1] + M[6] * wr[2] + M[7];

	return (uv[0] >= 0.0f && uv[1] >= 0.0f && uv[0] + uv[1] <= 1.0f);
}

inline PR_LIB bfloat intersectBW12(
	const RayPackage& in,
	const float* M,
	Vector2fv& uv,
	vfloat& t)
{
	const vfloat transS = M[8] * in.Origin[0] + M[9] * in.Origin[1] + M[10] * in.Origin[2] + M[11];
	const vfloat transD = M[8] * in.Direction[0] + M[9] * in.Direction[1] + M[10] * in.Direction[2];
	t					= -transS / transD;

	bfloat valid = (t > PR_TRIANGLE_BW_INTERSECT_EPSILON) & (abs(transD) > PR_EPSILON);
	if (none(valid))
		return valid;

	const Vector3fv wr = in.t(t);

	uv[0] = M[0] * wr[0] + M[1] * wr[1] + M[2] * wr[2] + M[3];
	uv[1] = M[4] * wr[0] + M[5] * wr[1] + M[6] * wr[2] + M[7];

	return valid & (uv[0] >= 0.0f) & (uv[1] >= 0.0f) & (uv[0] + uv[1] <= 1.0f);
}
} // namespace TriangleIntersection
} // namespace PR
