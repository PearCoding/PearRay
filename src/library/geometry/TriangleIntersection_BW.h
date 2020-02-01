#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_WT_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
/* Doug Baldwin and Michael Weber, Fast Ray-Triangle Intersections by Coordinate Transformation,
 * Journal of Computer Graphics Techniques (JCGT), vol. 5, no. 3, 39-49, 2016
 * http://jcgt.org/published/0005/03/03/
 */
inline PR_LIB bool intersectWB9(
	const Ray& in,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& p3,
	Vector2f& uv,
	float& t)
{
	return false;
}

inline PR_LIB bfloat intersectWB9(
	const RayPackage& in,
	const Vector3fv& p1,
	const Vector3fv& p2,
	const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& t)
{
	return bfloat(false);
}

inline PR_LIB bool intersectWB12(
	const Ray& in,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& p3,
	Vector2f& uv,
	float& t)
{
	return false;
}

inline PR_LIB bfloat intersectWB12(
	const RayPackage& in,
	const Vector3fv& p1,
	const Vector3fv& p2,
	const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& t)
{
	return bfloat(false);
}
} // namespace TriangleIntersection
} // namespace PR
