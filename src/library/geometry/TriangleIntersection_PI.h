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
	const Vector3f dR = in.Direction;
	const Vector3f mR = in.momentum();

	// Edge 1
	const Vector3f d0 = p1 - p0;
	const Vector3f m0 = p0.cross(p1);
	const float u	  = m0.dot(dR);
	const float s0	  = d0.dot(mR) + u;

	// Edge 2
	const Vector3f d2 = p0 - p2;
	const Vector3f m2 = p2.cross(p0);
	const float v	  = m2.dot(dR);
	const float s2	  = d2.dot(mR) + v;

	// Edge 3
	const Vector3f d1 = p2 - p1;
	const Vector3f m1 = p1.cross(p2);
	const float w	  = m1.dot(dR);
	const float s1	  = d1.dot(mR) + w;
	const bool valid  = (std::min(std::min(s0, s1), s2) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON)
					   || (std::max(std::max(s0, s1), s2) <= PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_LIKELY(!valid))
		return false;

	// Normal
	const float k = u + v + w;
	if (PR_UNLIKELY(abs(k) <= PR_EPSILON))
		return false;

	// Intersection value
	const Vector3f N = m0 + m1 + m2;
	t				 = (p0 - in.Origin).dot(N) / k;
	if (t <= PR_TRIANGLE_PI_INTERSECT_EPSILON)
		return false;

	// UV calculation!
	const float K = s0 + s1 + s2;
	uv(0)		  = s0 / K;
	uv(1)		  = s2 / K;

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
	const Vector3fv dR = in.Direction;
	const Vector3fv mR = in.momentum();

	// Edge 1
	const Vector3fv d0 = p1 - p0;
	const Vector3fv m0 = p0.cross(p1);
	const vfloat u	   = m0.dot(dR);
	const vfloat s0	   = d0.dot(mR) + u;

	// Edge 2
	const Vector3fv d2 = p0 - p2;
	const Vector3fv m2 = p2.cross(p0);
	const vfloat v	   = m2.dot(dR);
	const vfloat s2	   = d2.dot(mR) + v;

	// Edge 3
	const Vector3fv d1 = p2 - p1;
	const Vector3fv m1 = p1.cross(p2);
	const vfloat w	   = m1.dot(dR);
	const vfloat s1	   = d1.dot(mR) + w;

	bfloat valid = (min(min(s0, s1), s2) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON)
				   | (max(max(s0, s1), s2) <= PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Normal
	const vfloat k = u + v + w;
	valid		   = valid & (abs(k) > PR_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// Intersection value
	const Vector3fv N = m0 + m1 + m2;
	t				  = (p0 - in.Origin).dot(N) / k;
	valid			  = valid & (t > PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (none(valid))
		return valid;

	// UV calculation!
	const vfloat K = s0 + s1 + s2;
	uv(0)		   = s0 / K;
	uv(1)		   = s2 / K;

	return valid;
}

/////////////////////////////////////////////////////////////////////
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
	const Vector3f dR = in.Direction;
	const Vector3f mR = in.momentum();

	// Edge 1
	const Vector3f d0 = p1 - p0;
	const float u	  = m0.dot(dR);
	const float s0	  = d0.dot(mR) + u;

	// Edge 2
	const Vector3f d2 = p0 - p2;
	const float v	  = m2.dot(dR);
	const float s2	  = d2.dot(mR) + v;

	// Edge 3
	const Vector3f d1 = p2 - p1;
	const float w	  = m1.dot(dR);
	const float s1	  = d1.dot(mR) + w;

	const bool valid = (std::min(std::min(s0, s1), s2) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON)
					   || (std::max(std::max(s0, s1), s2) <= PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_LIKELY(!valid))
		return false;

	// Determinant
	const float k = u + v + w;
	if (PR_UNLIKELY(abs(k) <= PR_EPSILON))
		return false;

	// Intersection value
	const Vector3f N = m0 + m1 + m2;
	t				 = (p0 - in.Origin).dot(N) / k;
	if (t <= PR_TRIANGLE_PI_INTERSECT_EPSILON)
		return false;

	// UV calculation!
	const float K = s0 + s1 + s2;
	uv(0)		  = s0 / K;
	uv(1)		  = s2 / K;

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
	const Vector3fv dR = in.Direction;
	const Vector3fv mR = in.momentum();

	// Edge 1
	const Vector3fv d0 = p1 - p0;
	const vfloat u	   = m0.dot(dR);
	const vfloat s0	   = d0.dot(mR) + u;

	// Edge 2
	const Vector3fv d2 = p0 - p2;
	const vfloat v	   = m2.dot(dR);
	const vfloat s2	   = d2.dot(mR) + v;

	// Edge 3
	const Vector3fv d1 = p2 - p1;
	const vfloat w	   = m1.dot(dR);
	const vfloat s1	   = d1.dot(mR) + w;

	bfloat valid = (min(min(s0, s1), s2) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON)
				   | (max(max(s0, s1), s2) <= PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_LIKELY(none(valid)))
		return valid;

	// Normal
	const vfloat k = u + v + w;
	valid		   = valid & (abs(k) > PR_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	// Intersection value
	const Vector3fv N = m0 + m1 + m2;
	t				  = (p0 - in.Origin).dot(N) / k;
	valid			  = valid & (t > PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (none(valid))
		return valid;

	// UV calculation!
	const vfloat K = s0 + s1 + s2;
	uv(0)		   = s0 / K;
	uv(1)		   = s2 / K;

	return valid;
}

/////////////////////////////////////////////////////////////////////
// Pluecker based intersection method
// Intel Embree based variant (https://github.com/embree/embree)
inline PR_LIB bool intersectPI_Em(
	const Ray& in,
	const Vector3f& p0,
	const Vector3f& p1,
	const Vector3f& p2,
	Vector2f& uv,
	float& t)
{
	const Vector3f oR = in.Origin;
	const Vector3f dR = in.Direction;

	// Transform relative to origin for better precision
	const Vector3f lp0 = p0 - oR;
	const Vector3f lp1 = p1 - oR;
	const Vector3f lp2 = p2 - oR;

	// Edges (or pluecker distance vector)
	const Vector3f e0 = lp2 - lp0;
	const Vector3f e1 = lp0 - lp1;
	const Vector3f e2 = lp1 - lp2;

	// Edge tests
	const float u = e0.cross(lp2 + lp0).dot(dR);
	const float v = e1.cross(lp0 + lp1).dot(dR);
	const float w = e2.cross(lp1 + lp2).dot(dR);

	if (!(std::min(std::min(u, v), w) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON
		  || std::max(std::max(u, v), w) <= PR_TRIANGLE_PI_INTERSECT_EPSILON))
		return false;

	const Vector3f N = -e0.cross(e1);
	const float den	 = 2 * N.dot(dR);
	if (abs(den) <= PR_TRIANGLE_PI_INTERSECT_EPSILON)
		return false;

	const float lt = 2 * lp0.dot(N);
	t			   = lt / den;

	if (t <= PR_TRIANGLE_PI_INTERSECT_EPSILON)
		return false;

	uv = Vector2f(u, v) / (u + v + w);
	return true;
}

inline PR_LIB bfloat intersectPI_Em(
	const RayPackage& in,
	const Vector3fv& p0,
	const Vector3fv& p1,
	const Vector3fv& p2,
	Vector2fv& uv,
	vfloat& t)
{
	const Vector3fv oR = in.Origin;
	const Vector3fv dR = in.Direction;

	// Transform relative to origin for better precision
	const Vector3fv lp0 = p0 - oR;
	const Vector3fv lp1 = p1 - oR;
	const Vector3fv lp2 = p2 - oR;

	// Edges (or pluecker distance vector)
	const Vector3fv e0 = lp2 - lp0;
	const Vector3fv e1 = lp0 - lp1;
	const Vector3fv e2 = lp1 - lp2;

	// Edge tests
	const vfloat u = e0.cross(lp2 + lp0).dot(dR);
	const vfloat v = e1.cross(lp0 + lp1).dot(dR);
	const vfloat w = e2.cross(lp1 + lp2).dot(dR);

	bfloat valid = min(min(u, v), w) >= -PR_TRIANGLE_PI_INTERSECT_EPSILON;
	valid		 = valid | (max(max(u, v), w) <= PR_TRIANGLE_PI_INTERSECT_EPSILON);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	const Vector3fv N = -e0.cross(e1);
	const vfloat den  = 2 * N.dot(dR);
	const vfloat lt	  = 2 * lp0.dot(N);
	t				  = lt / den;
	const vfloat UVW  = (u + v + w);
	uv[0]			  = u / UVW;
	uv[1]			  = v / UVW;
	return valid & (den != 0) & (t > PR_TRIANGLE_PI_INTERSECT_EPSILON);
	;
}
} // namespace TriangleIntersection
} // namespace PR
