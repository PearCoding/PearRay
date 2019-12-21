#pragma once

#include "math/SIMD.h"

#define PR_TRIANGLE_INTERSECT_EPSILON (1e-4f)
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
	const float a	= q.dot(e12);

	if (abs(a) < PR_EPSILON)
		return false;

	const float f	= 1.0f / a;
	const Vector3f s = in.Origin - p1;
	uv(0)			 = f * s.dot(q);

	if (uv(0) < 0 || uv(0) > 1)
		return false;

	const Vector3f r = s.cross(e12);
	uv(1)			 = f * in.Direction.dot(r);

	if (uv(1) < 0 || uv(0) + uv(1) > 1)
		return false;

	t = f * r.dot(e13);
	return t >= PR_TRIANGLE_INTERSECT_EPSILON;
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
	const vfloat a	= q.dot(e12);

	const vfloat f	= 1.0f / a;
	const Vector3fv s = in.Origin - p1;
	uv(0)			  = f * s.dot(q);

	const bfloat uoutside = (uv(0) < 0) | (uv(0) > 1);

	const Vector3fv r = s.cross(e12);
	uv(1)			  = f * in.Direction.dot(r);

	const bfloat voutside = (uv(1) < 0) | (uv(0) + uv(1) > 1);

	t = f * r.dot(e13);
	return b_and(b_and(b_not(uoutside), b_not(voutside)),
				 (t >= PR_TRIANGLE_INTERSECT_EPSILON));
}
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
	Vector3f d0 = p0 - p1;
	Vector3f m0 = p0.cross(p1);
	float s0	= d0.dot(mR) + m0.dot(dR);
	bool sign   = std::signbit(s0);

	// Edge 2
	Vector3f d2 = p2 - p0;
	Vector3f m2 = p2.cross(p0);
	float s2	= d2.dot(mR) + m2.dot(dR);
	if (std::signbit(s2) != sign)
		return false;

	// Edge 3
	Vector3f d1 = p1 - p2;
	Vector3f m1 = p1.cross(p2);
	float s1	= d1.dot(mR) + m1.dot(dR);
	if (std::signbit(s1) != sign)
		return false;

	// Normal
	Vector3f N = -d0.cross(d2);
	float k	= dR.dot(N);
	if (std::abs(k) <= PR_EPSILON)
		return false;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	if (t <= PR_TRIANGLE_INTERSECT_EPSILON)
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
	Vector3fv d0 = p0 - p1;
	Vector3fv m0 = p0.cross(p1);
	vfloat s0	= d0.dot(mR) + m0.dot(dR);
	vfloat si	= sign(s0);

	// Edge 2
	Vector3fv d2 = p2 - p0;
	Vector3fv m2 = p2.cross(p0);
	vfloat s2	= d2.dot(mR) + m2.dot(dR);
	bfloat neq1   = b_not(b_and(sign(s2),si));

	// Edge 3
	Vector3fv d1 = p1 - p2;
	Vector3fv m1 = p1.cross(p2);
	vfloat s1	= d1.dot(mR) + m1.dot(dR);
	bfloat neq2   = (sign(s1) != si);

	// Normal
	Vector3fv N = -d0.cross(d2);
	vfloat k	= dR.dot(N);
	bfloat kiv  = (abs(k) <= PR_EPSILON);

	// Intersection value
	t			  = (p0 - in.Origin).dot(N) / k;
	bfloat behind = (t <= PR_TRIANGLE_INTERSECT_EPSILON);

	// UV calculation!
	Vector3fv p = in.t(t);
	vfloat a2   = N.squaredNorm();

	uv(0) = sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return b_not(b_or(behind, b_or(kiv, b_or(neq1, neq2))));
}

// Pluecker based intersection method
// Optimized variant -> Everything is cached!
inline PR_LIB bool intersectPI_Opt(
	const Ray& in,
	const Vector3f& p0,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& N,
	const Vector3f& m0,
	const Vector3f& m1,
	const Vector3f& m2,
	Vector2f& uv,
	float& t)
{
	Vector3f dR = in.Direction;
	Vector3f mR = in.momentum();

	// Edge 1
	Vector3f d0 = p0 - p1;
	//Vector3f m0 = p0.cross(p1);
	float s0  = d0.dot(mR) + m0.dot(dR);
	bool sign = std::signbit(s0);

	// Edge 2
	Vector3f d2 = p2 - p0;
	//Vector3f m2 = p2.cross(p0);
	float s2 = d2.dot(mR) + m2.dot(dR);
	if (std::signbit(s2) != sign)
		return false;

	// Edge 3
	Vector3f d1 = p1 - p2;
	//Vector3f m1 = p1.cross(p2);
	float s1 = d1.dot(mR) + m1.dot(dR);
	if (std::signbit(s1) != sign)
		return false;

	// Normal
	//Vector3f N = -d0.cross(d2);
	float k = dR.dot(N);
	if (std::abs(k) <= PR_EPSILON)
		return false;

	// Intersection value
	t = (p0 - in.Origin).dot(N) / k;
	if (t <= PR_TRIANGLE_INTERSECT_EPSILON)
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
	const Vector3fv& N,
	const Vector3fv& m0,
	const Vector3fv& m1,
	const Vector3fv& m2,
	Vector2fv& uv,
	vfloat& t)
{
	Vector3fv dR = in.Direction;
	Vector3fv mR = in.momentum();

	// Edge 1
	Vector3fv d0 = p0 - p1;
	vfloat s0	= d0.dot(mR) + m0.dot(dR);
	vfloat si	= sign(s0);

	// Edge 2
	Vector3fv d2 = p2 - p0;
	vfloat s2	= d2.dot(mR) + m2.dot(dR);
	bfloat neq1  = (sign(s2) != si);

	// Edge 3
	Vector3fv d1 = p1 - p2;
	vfloat s1	= d1.dot(mR) + m1.dot(dR);
	bfloat neq2  = (sign(s1) != si);

	// Normal
	vfloat k   = dR.dot(N);
	bfloat kiv = (abs(k) <= PR_EPSILON);

	// Intersection value
	t			  = (p0 - in.Origin).dot(N) / k;
	bfloat behind = (t <= PR_TRIANGLE_INTERSECT_EPSILON);

	// UV calculation!
	Vector3fv p = in.t(t);
	vfloat a2   = N.squaredNorm();

	uv(0) = sqrt((p2 - p).cross(p0 - p).squaredNorm() / a2);
	uv(1) = sqrt((p0 - p).cross(p1 - p).squaredNorm() / a2);

	return b_not(b_or(behind, b_or(kiv, b_or(neq1, neq2))));
}
} // namespace TriangleIntersection
} // namespace PR
