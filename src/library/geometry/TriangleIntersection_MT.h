#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_MT_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
// Moeller-Trumbore
namespace details {
inline PR_LIB bool _intersectBaseMT(
	const Ray& in,
	const Vector3f& p1, const Vector3f& p2, const Vector3f& p3,
	Vector2f& uv,
	float& f, Vector3f& r, Vector3f& e13)
{
	const Vector3f e12 = p2 - p1;
	e13				   = p3 - p1;

	const Vector3f q = in.Direction.cross(e13);
	const float a	 = q.dot(e12);

	if (PR_UNLIKELY(abs(a) <= PR_EPSILON))
		return false;

	f				 = 1.0f / a;
	const Vector3f s = in.Origin - p1;
	uv(0)			 = f * s.dot(q);

	if (PR_LIKELY(uv(0) < 0.0f || uv(0) - 1.0f > PR_EPSILON))
		return false;

	r	  = s.cross(e12);
	uv(1) = f * in.Direction.dot(r);

	return (uv(1) >= 0.0f && uv(0) + uv(1) - 1.0f <= PR_EPSILON);
}

inline PR_LIB bool _intersectBaseMT(
	const Ray& in,
	const Vector3f& p1, const Vector3f& p2, const Vector3f& p3,
	Vector2f& uv,
	float& t)
{
	float f;
	Vector3f r, e13;
	if (!_intersectBaseMT(in, p1, p2, p3, uv, f, r, e13))
		return false;
	t = f * r.dot(e13);
	return in.isInsideRange(t);
}

inline PR_LIB bfloat _intersectBaseMT(
	const RayPackage& in,
	const Vector3fv& p1, const Vector3fv& p2, const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& f, Vector3fv& r, Vector3fv& e13)
{
	using namespace simdpp;

	const Vector3fv e12 = p2 - p1;
	e13					= p3 - p1;

	const Vector3fv q = in.Direction.cross(e13);
	const vfloat a	  = q.dot(e12);

	bfloat valid = abs(a) > PR_EPSILON;
	if (PR_UNLIKELY(none(valid)))
		return valid;

	f				  = 1.0f / a;
	const Vector3fv s = in.Origin - p1;
	uv(0)			  = f * s.dot(q);

	valid = valid & ((uv(0) >= 0.0f) & (uv(0) - vfloat(1.0f) <= PR_EPSILON));
	if (PR_LIKELY(none(valid)))
		return valid;

	r	  = s.cross(e12);
	uv(1) = f * in.Direction.dot(r);

	return valid & (uv(1) >= 0.0f) & (uv(0) + uv(1) - vfloat(1.0f) <= PR_EPSILON);
}

inline PR_LIB bfloat _intersectBaseMT(
	const RayPackage& in,
	const Vector3fv& p1, const Vector3fv& p2, const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& t)
{
	vfloat f;
	Vector3fv r, e13;
	bfloat valid = _intersectBaseMT(in, p1, p2, p3, uv, f, r, e13);
	t			 = f * r.dot(e13);
	return valid & in.isInsideRange(t);
}
} // namespace details

inline PR_LIB bool intersectLineMT(
	const Ray& in,
	const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
{
	Vector2f uv;
	float f;
	Vector3f r, e13;
	return details::_intersectBaseMT(in, p1, p2, p3, uv, f, r, e13);
}

inline PR_LIB bool intersectMT(
	const Ray& in,
	const Vector3f& p1, const Vector3f& p2, const Vector3f& p3,
	float& t)
{
	Vector2f uv;
	return details::_intersectBaseMT(in, p1, p2, p3, uv, t);
}

inline PR_LIB bool intersectMT(
	const Ray& in,
	const Vector3f& p1, const Vector3f& p2, const Vector3f& p3,
	float& t, Vector2f& uv)
{
	return details::_intersectBaseMT(in, p1, p2, p3, uv, t);
}

inline PR_LIB bfloat intersectLineMT(
	const RayPackage& in,
	const Vector3fv& p1, const Vector3fv& p2, const Vector3fv& p3)
{
	Vector2fv uv;
	vfloat f;
	Vector3fv r, e13;
	return details::_intersectBaseMT(in, p1, p2, p3, uv, f, r, e13);
}

inline PR_LIB bfloat intersectMT(
	const RayPackage& in,
	const Vector3fv& p1, const Vector3fv& p2, const Vector3fv& p3,
	vfloat& t)
{
	Vector2fv uv;
	return details::_intersectBaseMT(in, p1, p2, p3, uv, t);
}

inline PR_LIB bfloat intersectMT(
	const RayPackage& in,
	const Vector3fv& p1, const Vector3fv& p2, const Vector3fv& p3,
	vfloat& t, Vector2fv& uv)
{
	return details::_intersectBaseMT(in, p1, p2, p3, uv, t);
}
} // namespace TriangleIntersection
} // namespace PR
