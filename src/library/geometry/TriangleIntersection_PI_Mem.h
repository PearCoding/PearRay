#pragma once

#include "ray/RayPackage.h"

namespace PR {
namespace TriangleIntersection {
/////////////////////////////////////////////////////////////////////
// Pluecker based intersection method
// Optimized variant -> Everything is cached!

namespace details {
inline PR_LIB bool _intersectBasePI_Mem(
	const Ray& in,
	const Vector3f& p0,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& m0,
	const Vector3f& m1,
	const Vector3f& m2,
	Vector3f& d0, Vector3f& d2,
	float& s0, float& s1, float& s2)
{
	const Vector3f dR = in.Direction;
	const Vector3f mR = in.momentum();

	// Edge 1
	d0			   = p1 - p0;
	const float k0 = m0.dot(dR);
	s0			   = d0.dot(mR) + k0;

	// Edge 2
	d2			   = p0 - p2;
	const float k2 = m2.dot(dR);
	s2			   = d2.dot(mR) + k2;

	// Edge 3
	const Vector3f d1 = p2 - p1;
	const float k1	  = m1.dot(dR);
	s1				  = d1.dot(mR) + k1;

	return (std::min(std::min(s0, s1), s2) > -PR_EPSILON)
		   || (std::max(std::max(s0, s1), s2) < PR_EPSILON);
}

inline PR_LIB bool _intersectBasePI_Mem(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	const Vector3f& m0, const Vector3f& m1, const Vector3f& m2,
	Vector3f& d0, Vector3f& d2,
	float& s0, float& s1, float& s2,
	float& t)
{
	const bool valid = _intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2);
	if (PR_LIKELY(!valid))
		return false;

	// Determinant
	const Vector3f N = d2.cross(d0);
	const float k	 = N.dot(in.Direction);
	if (PR_UNLIKELY(abs(k) <= PR_EPSILON))
		return false;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	return in.isInsideRange(t);
}

///////////////// Vector variant
inline PR_LIB bfloat _intersectBasePI_Mem(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	const Vector3fv& m0, const Vector3fv& m1, const Vector3fv& m2,
	Vector3fv& d0, Vector3fv& d2,
	vfloat& s0, vfloat& s1, vfloat& s2)
{
	const Vector3fv dR = in.Direction;
	const Vector3fv mR = in.momentum();

	// Edge 1
	d0				= p1 - p0;
	const vfloat k0 = m0.dot(dR);
	s0				= d0.dot(mR) + k0;

	// Edge 2
	d2				= p0 - p2;
	const vfloat k2 = m2.dot(dR);
	s2				= d2.dot(mR) + k2;

	// Edge 3
	const Vector3fv d1 = p2 - p1;
	const vfloat k1	   = m1.dot(dR);
	s1				   = d1.dot(mR) + k1;

	return (min(min(s0, s1), s2) > -PR_EPSILON)
		   | (max(max(s0, s1), s2) < PR_EPSILON);
}

inline PR_LIB bfloat _intersectBasePI_Mem(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	const Vector3fv& m0, const Vector3fv& m1, const Vector3fv& m2,
	Vector3fv& d0, Vector3fv& d2,
	vfloat& s0, vfloat& s1, vfloat& s2,
	vfloat& t)
{
	bfloat valid = _intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Normal
	const Vector3fv N = d2.cross(d0);
	const vfloat k	  = N.dot(in.Direction);
	valid			  = valid & (abs(k) > PR_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	return valid & in.isInsideRange(t);
}
} // namespace details

inline PR_LIB bool intersectLinePI_Mem(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	const Vector3f& m0, const Vector3f& m1, const Vector3f& m2)
{
	Vector3f d0, d2;
	float s0, s1, s2;
	return details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2);
}

inline PR_LIB bool intersectPI_Mem(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	const Vector3f& m0, const Vector3f& m1, const Vector3f& m2,
	float& t)
{
	Vector3f d0, d2;
	float s0, s1, s2;
	return details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2, t);
}

inline PR_LIB bool intersectPI_Mem(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	const Vector3f& m0, const Vector3f& m1, const Vector3f& m2,
	float& t, Vector2f& uv)
{
	Vector3f d0, d2;
	float s0, s1, s2;
	const bool valid = details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2, t);
	if (!valid)
		return false;

	// UV calculation!
	const float K = s0 + s1 + s2;
	uv(0)		  = s2 / K;
	uv(1)		  = s0 / K;

	return true;
}

inline PR_LIB bfloat intersectLinePI_Mem(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	const Vector3fv& m0, const Vector3fv& m1, const Vector3fv& m2)
{
	Vector3fv d0, d2;
	vfloat s0, s1, s2;
	return details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2);
}

inline PR_LIB bfloat intersectPI_Mem(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	const Vector3fv& m0, const Vector3fv& m1, const Vector3fv& m2,
	vfloat& t)
{
	Vector3fv d0, d2;
	vfloat s0, s1, s2;
	return details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2, t);
}

inline PR_LIB bfloat intersectPI_Mem(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	const Vector3fv& m0, const Vector3fv& m1, const Vector3fv& m2,
	vfloat& t, Vector2fv& uv)
{
	Vector3fv d0, d2;
	vfloat s0, s1, s2;
	bfloat valid = details::_intersectBasePI_Mem(in, p0, p1, p2, m0, m1, m2, d0, d2, s0, s1, s2, t);
	if (none(valid))
		return valid;

	// UV calculation!
	const vfloat K = s0 + s1 + s2;
	uv(0)		   = s2 / K;
	uv(1)		   = s0 / K;

	return valid;
}
} // namespace TriangleIntersection
} // namespace PR
