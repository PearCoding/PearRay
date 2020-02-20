#pragma once

#include "ray/RayPackage.h"

namespace PR {
namespace TriangleIntersection {
/////////////////////////////////////////////////////////////////////
// Pluecker based intersection method
// Intel Embree based offset variant (https://github.com/embree/embree)
namespace details {
inline PR_LIB bool _intersectBasePI_Off(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	Vector3f& e0, Vector3f& e1,
	float& u, float& v, float& w)
{
	const Vector3f oR = in.Origin;
	const Vector3f dR = in.Direction;

	// Transform relative to origin for better precision
	const Vector3f lp0 = p0 - oR;
	const Vector3f lp1 = p1 - oR;
	const Vector3f lp2 = p2 - oR;

	// Edges (or pluecker distance vector)
	e0				  = lp2 - lp0;
	e1				  = lp0 - lp1;
	const Vector3f e2 = lp1 - lp2;

	// Edge tests
	u = e0.cross(lp2 + lp0).dot(dR);
	v = e1.cross(lp0 + lp1).dot(dR);
	w = e2.cross(lp1 + lp2).dot(dR);

	return (std::min(std::min(u, v), w) >= -PR_EPSILON || std::max(std::max(u, v), w) <= PR_EPSILON);
}

inline PR_LIB bool _intersectBasePI_Off(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	Vector3f& e0, Vector3f& e1,
	float& u, float& v, float& w,
	float& t)
{
	if (!_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w))
		return false;

	const Vector3f N = -e0.cross(e1);
	const float den	 = N.dot(in.Direction);
	if (abs(den) <= PR_EPSILON)
		return false;

	const float lt = (p0 - in.Origin).dot(N);
	t			   = lt / den;

	return in.isInsideRange(t);
}

inline PR_LIB bfloat _intersectBasePI_Off(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	Vector3fv& e0, Vector3fv& e1,
	vfloat& u, vfloat& v, vfloat& w)
{
	const Vector3fv oR = in.Origin;
	const Vector3fv dR = in.Direction;

	// Transform relative to origin for better precision
	const Vector3fv lp0 = p0 - oR;
	const Vector3fv lp1 = p1 - oR;
	const Vector3fv lp2 = p2 - oR;

	// Edges (or pluecker distance vector)
	e0				   = lp2 - lp0;
	e1				   = lp0 - lp1;
	const Vector3fv e2 = lp1 - lp2;

	// Edge tests
	u = e0.cross(lp2 + lp0).dot(dR);
	v = e1.cross(lp0 + lp1).dot(dR);
	w = e2.cross(lp1 + lp2).dot(dR);

	return (min(min(u, v), w) >= -PR_EPSILON)
		   | (max(max(u, v), w) <= PR_EPSILON);
}

inline PR_LIB bfloat _intersectBasePI_Off(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	Vector3fv& e0, Vector3fv& e1,
	vfloat& u, vfloat& v, vfloat& w,
	vfloat& t)
{
	const bfloat valid = _intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w);
	if (PR_UNLIKELY(none(valid)))
		return valid;

	const Vector3fv N = -e0.cross(e1);
	const vfloat den  = N.dot(in.Direction);
	const vfloat lt	  = (p0 - in.Origin).dot(N);
	t				  = lt / den;
	return valid & (den != 0) & in.isInsideRange(t);
}
////////////////// Vector Variant
} // namespace details

inline PR_LIB bool intersectLinePI_Off(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2)
{
	Vector3f e0, e1;
	float u, v, w;

	return details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w);
}

inline PR_LIB bool intersectPI_Off(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	float& t)
{
	Vector3f e0, e1;
	float u, v, w;

	return details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w, t);
}

inline PR_LIB bool intersectPI_Off(
	const Ray& in,
	const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
	float& t, Vector2f& uv)
{
	Vector3f e0, e1;
	float u, v, w;

	if (!details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w, t))
		return false;

	uv = Vector2f(u, v) / (u + v + w);
	return true;
}

inline PR_LIB bfloat intersectLinePI_Off(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2)
{
	Vector3fv e0, e1;
	vfloat u, v, w;

	return details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w);
}

inline PR_LIB bfloat intersectPI_Off(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	vfloat& t)
{
	Vector3fv e0, e1;
	vfloat u, v, w;

	return details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w, t);
}

inline PR_LIB bfloat intersectPI_Off(
	const RayPackage& in,
	const Vector3fv& p0, const Vector3fv& p1, const Vector3fv& p2,
	vfloat& t, Vector2fv& uv)
{
	Vector3fv e0, e1;
	vfloat u, v, w;

	const bfloat valid = details::_intersectBasePI_Off(in, p0, p1, p2, e0, e1, u, v, w, t);
	const vfloat UVW   = u + v + w;
	uv[0]			   = u / UVW;
	uv[1]			   = v / UVW;
	return valid;
}
} // namespace TriangleIntersection
} // namespace PR
