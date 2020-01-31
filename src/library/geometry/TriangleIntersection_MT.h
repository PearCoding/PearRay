#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_MT_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
// Moeller-Trumbore
inline PR_LIB bool intersectMT(
	const Ray& in,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& p3,
	Vector2f& uv,
	float& t)
{
	const Vector3f e12 = p2 - p1;
	const Vector3f e13 = p3 - p1;

	const Vector3f q = in.Direction.cross(e13);
	const float a	 = q.dot(e12);

	if (PR_UNLIKELY(abs(a) <= PR_EPSILON))
		return false;

	const float f	 = 1.0f / a;
	const Vector3f s = in.Origin - p1;
	uv(0)			 = f * s.dot(q);

	if (PR_LIKELY(uv(0) < 0 || uv(0) > 1))
		return false;

	const Vector3f r = s.cross(e12);
	uv(1)			 = f * in.Direction.dot(r);

	if (PR_LIKELY(uv(1) < 0 || uv(0) + uv(1) > 1))
		return false;

	t = f * r.dot(e13);
	return t >= PR_TRIANGLE_MT_INTERSECT_EPSILON;
}

inline PR_LIB bfloat intersectMT(
	const RayPackage& in,
	const Vector3fv& p1,
	const Vector3fv& p2,
	const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& t)
{
	using namespace simdpp;

	const Vector3fv e12 = p2 - p1;
	const Vector3fv e13 = p3 - p1;

	const Vector3fv q = in.Direction.cross(e13);
	const vfloat a	  = q.dot(e12);

	bfloat valid = abs(a) > PR_EPSILON;
	if (PR_UNLIKELY(none(valid)))
		return valid;

	const vfloat f	  = 1.0f / a;
	const Vector3fv s = in.Origin - p1;
	uv(0)			  = f * s.dot(q);

	valid = valid & ((uv(0) >= 0) & (uv(0) <= 1));
	if (PR_LIKELY(none(valid)))
		return valid;

	const Vector3fv r = s.cross(e12);
	uv(1)			  = f * in.Direction.dot(r);

	valid = valid & ((uv(1) >= 0) & (uv(0) + uv(1) <= 1));

	t = f * r.dot(e13);
	return valid & (t >= PR_TRIANGLE_MT_INTERSECT_EPSILON);
}
} // namespace TriangleIntersection
} // namespace PR
