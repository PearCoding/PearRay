#pragma once

#include "ray/RayPackage.h"

#define PR_TRIANGLE_WT_INTERSECT_EPSILON (PR_EPSILON)
namespace PR {
namespace TriangleIntersection {
// Watertight (Woop, Benthin, Wald) 2013
inline PR_LIB bool intersectWT(
	const Ray& in,
	const Vector3f& p1,
	const Vector3f& p2,
	const Vector3f& p3,
	Vector2f& uv,
	float& t)
{
	const uint32 kz = in.maxDirectionIndex();
	uint32 kx		= kz + 1;
	if (kx == 3)
		kx = 0;
	uint32 ky = kx + 1;
	if (ky == 3)
		ky = 0;

	if (in.Direction[kz] < 0)
		std::swap(kx, ky);

	const float dX = in.Direction[kx];
	const float dY = in.Direction[ky];
	const float dZ = in.Direction[kz];

	const float sx = dX / dZ;
	const float sy = dY / dZ;
	const float sz = 1.0f / dZ;

	// We use (1-u-v)*P1 + u*P2 + v*P3 convention
	Eigen::Vector3f A = p2 - in.Origin;
	Eigen::Vector3f B = p3 - in.Origin;
	Eigen::Vector3f C = p1 - in.Origin;

	// Shear
	const float Ax = A(kx) - sx * A(kz);
	const float Ay = A(ky) - sy * A(kz);
	const float Bx = B(kx) - sx * B(kz);
	const float By = B(ky) - sy * B(kz);
	const float Cx = C(kx) - sx * C(kz);
	const float Cy = C(ky) - sy * C(kz);

	float u = Cx * By - Cy * Bx;
	float v = Ax * Cy - Ay * Cx;
	float w = Bx * Ay - By * Ax;

	// Better precision needed:
	if (u <= PR_EPSILON || v <= PR_EPSILON || w <= PR_EPSILON) {
		double CxBy = (double)Cx * (double)By;
		double CyBx = (double)Cy * (double)Bx;
		u			= (float)(CxBy - CyBx);

		double AxCy = (double)Ax * (double)Cy;
		double AyCx = (double)Ay * (double)Cx;
		v			= (float)(AxCy - AyCx);

		double BxAy = (double)Bx * (double)Ay;
		double ByAx = (double)By * (double)Ax;
		w			= (float)(BxAy - ByAx);
	}

	if ((u < 0 || v < 0 || w < 0) && (u > 0 || v > 0 || w > 0))
		return false;

	const float det = u + v + w;
	if (std::abs(det) < PR_TRIANGLE_WT_INTERSECT_EPSILON)
		return false;

	const float Az = sz * A(kz);
	const float Bz = sz * B(kz);
	const float Cz = sz * C(kz);

	t = u * Az + v * Bz + w * Cz;
	if (std::abs(t) >= PR_TRIANGLE_WT_INTERSECT_EPSILON && std::signbit(t) == std::signbit(det)) {
		const float invDet = 1.0f / det;
		uv[0]			   = u * invDet;
		uv[1]			   = v * invDet;
		//w *= invDet;
		t *= invDet;

		return (t >= PR_TRIANGLE_WT_INTERSECT_EPSILON);
	}

	return false;
}

inline PR_LIB bfloat intersectWT(
	const RayPackage& /*in*/,
	const Vector3fv& /*p1*/,
	const Vector3fv& /*p2*/,
	const Vector3fv& /*p3*/,
	Vector2fv& /*uv*/,
	vfloat& t)
{
	// TODO
	return (t >= PR_TRIANGLE_WT_INTERSECT_EPSILON);
}
} // namespace TriangleIntersection
} // namespace PR
