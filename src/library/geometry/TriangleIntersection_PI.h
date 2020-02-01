#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_PI_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
// Pluecker based intersection method
// Non Optimized variant -> Nothing is cached!
inline PR_LIB bool intersectPI_NonOpt(
	const Ray& in,
	const Vector3f& p0,
	const Vector3f& p1,
	const Vector3f& p2,
	Vector2f& uv,
	float& t)
{
	Vector3f dR = in.Direction;
	Vector3f mR = in.momentum();

	// Edge 1
	Vector3f d0 = p1 - p0;
	Vector3f m0 = p0.cross(p1);
	float s0	= d0.dot(mR) + m0.dot(dR);
	bool sign	= std::signbit(s0);

	// Edge 2
	Vector3f d2 = p0 - p2;
	Vector3f m2 = p2.cross(p0);
	float s2	= d2.dot(mR) + m2.dot(dR);
	if (PR_LIKELY(std::signbit(s2) != sign))
		return false;

	// Edge 3
	Vector3f d1 = p2 - p1;
	Vector3f m1 = p1.cross(p2);
	float s1	= d1.dot(mR) + m1.dot(dR);
	if (PR_LIKELY(std::signbit(s1) != sign))
		return false;

	// Normal
	Vector3f N = m0 + m1 + m2;
	float k	   = dR.dot(N);
	if (PR_LIKELY(std::abs(k) <= PR_EPSILON))
		return false;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	if (PR_LIKELY(t <= PR_TRIANGLE_PI_INTERSECT_EPSILON))
		return false;

	// UV calculation!
	Vector3f p = in.t(t);
	float a2   = N.squaredNorm();

	uv(0) = std::sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = std::sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return true;
}

inline PR_LIB bfloat intersectPI_NonOpt(
	const RayPackage& in,
	const Vector3fv& p0,
	const Vector3fv& p1,
	const Vector3fv& p2,
	Vector2fv& uv,
	vfloat& t)
{
	Vector3fv dR = in.Direction;
	Vector3fv mR = in.momentum();

	// Edge 1
	Vector3fv d0 = p1 - p0;
	Vector3fv m0 = p0.cross(p1);
	vfloat s0	 = d0.dot(mR) + m0.dot(dR);
	bfloat si	 = signbit(s0);

	// Edge 2
	Vector3fv d2 = p0 - p2;
	Vector3fv m2 = p2.cross(p0);
	vfloat s2	 = d2.dot(mR) + m2.dot(dR);
	bfloat valid = ~(signbit(s2) ^ si);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Edge 3
	Vector3fv d1 = p2 - p1;
	Vector3fv m1 = p1.cross(p2);
	vfloat s1	 = d1.dot(mR) + m1.dot(dR);
	valid		 = valid & ~(signbit(s1) ^ si);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Normal
	Vector3fv N = m0 + m1 + m2;
	vfloat k	= dR.dot(N);
	valid		= valid & (abs(k) > PR_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// Intersection value
	t	  = (p0 - in.Origin).dot(N) / k;
	valid = valid & (t > PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// UV calculation!
	Vector3fv p = in.t(t);
	vfloat a2	= N.squaredNorm();

	uv(0) = sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return valid;
}

// Pluecker based intersection method
// Optimized variant -> Everything is cached!
inline PR_LIB bool intersectPI_Opt(
	const Ray& in,
	const Vector3f& p0,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& m0,
	const Vector3f& m1,
	const Vector3f& m2,
	Vector2f& uv,
	float& t)
{
	Vector3f dR = in.Direction;
	Vector3f mR = in.momentum();

	// Edge 1
	Vector3f d0 = p1 - p0;
	float s0  = d0.dot(mR) + m0.dot(dR);
	bool sign = std::signbit(s0);

	// Edge 2
	Vector3f d2 = p0 - p2;
	float s2 = d2.dot(mR) + m2.dot(dR);
	if (PR_LIKELY(std::signbit(s2) != sign))
		return false;

	// Edge 3
	Vector3f d1 = p2 - p1;
	float s1 = d1.dot(mR) + m1.dot(dR);
	if (PR_LIKELY(std::signbit(s1) != sign))
		return false;

	// Normal
	Vector3f N = m0 + m1 + m2;
	float k	   = dR.dot(N);
	if (PR_UNLIKELY(std::abs(k) <= PR_EPSILON))
		return false;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	if (PR_UNLIKELY(t <= PR_TRIANGLE_PI_INTERSECT_EPSILON))
		return false;

	// UV calculation!
	Vector3f p = in.t(t);
	float a2   = N.squaredNorm();

	uv(0) = std::sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = std::sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return true;
}

inline PR_LIB bfloat intersectPI_Opt(
	const RayPackage& in,
	const Vector3fv& p0,
	const Vector3fv& p1,
	const Vector3fv& p2,
	const Vector3fv& m0,
	const Vector3fv& m1,
	const Vector3fv& m2,
	Vector2fv& uv,
	vfloat& t)
{
	Vector3fv dR = in.Direction;
	Vector3fv mR = in.momentum();

	// Edge 1
	Vector3fv d0 = p1 - p0;
	vfloat s0	 = d0.dot(mR) + m0.dot(dR);
	bfloat si	 = signbit(s0);

	// Edge 2
	Vector3fv d2 = p0 - p2;
	vfloat s2	 = d2.dot(mR) + m2.dot(dR);
	bfloat valid = ~(signbit(s2) ^ si);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Edge 3
	Vector3fv d1 = p2 - p1;
	vfloat s1	 = d1.dot(mR) + m1.dot(dR);
	valid		 = valid & ~(signbit(s1) ^ si);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Normal
	Vector3fv N = m0 + m1 + m2;
	vfloat k	= dR.dot(N);
	valid		= valid & (abs(k) > PR_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// Intersection value
	t	  = (p0 - in.Origin).dot(N) / k;
	valid = valid & (t > PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// UV calculation!
	Vector3fv p = in.t(t);
	vfloat a2	= N.squaredNorm();

	uv(0) = sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return valid;
}
} // namespace TriangleIntersection
} // namespace PR