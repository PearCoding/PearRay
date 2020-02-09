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

	const float dZ = in.Direction[kz];
	if (dZ < 0)
		std::swap(kx, ky);

	const float dX = in.Direction[kx];
	const float dY = in.Direction[ky];

	const float sx = dX / dZ;
	const float sy = dY / dZ;
	const float sz = 1.0f / dZ;

	// We use (1-u-v)*P1 + u*P2 + v*P3 convention
	Vector3f A = p2 - in.Origin;
	Vector3f B = p3 - in.Origin;
	Vector3f C = p1 - in.Origin;

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
	if (u == 0.0f || v == 0.0f || w == 0.0f) {
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

	const bool invalid = (std::min(std::min(u, v), w) < -PR_TRIANGLE_WT_INTERSECT_EPSILON)
						 && (std::max(std::max(u, v), w) > PR_TRIANGLE_WT_INTERSECT_EPSILON);
	if (PR_LIKELY(invalid))
		return false;

	const float det = u + v + w;
	if (std::abs(det) < PR_TRIANGLE_WT_INTERSECT_EPSILON)
		return false;

	const float Az = sz * A(kz);
	const float Bz = sz * B(kz);
	const float Cz = sz * C(kz);

	t = u * Az + v * Bz + w * Cz;
	if (std::signbit(t) == std::signbit(det)) {
		const float invDet = 1.0f / det;
		uv[0]			   = u * invDet;
		uv[1]			   = v * invDet;
		//w *= invDet;
		t *= invDet;

		return in.isInsideRange(t);
	}

	return false;
}

inline PR_LIB bfloat intersectWT(
	const RayPackage& in,
	const Vector3fv& p1,
	const Vector3fv& p2,
	const Vector3fv& p3,
	Vector2fv& uv,
	vfloat& t)
{
	// TODO: A better swizzling algorithm would work wonders here...
	auto pick = [](const Vector3fv& a, const vuint32& idx) -> vfloat {
		return blend(a(0), blend(a(1), a(2), idx == 1), idx == 0);
	};

	const vuint32 kz = in.maxDirectionIndex();
	vuint32 kx		 = blend(kz + 1, vuint32(0), kz < 2);
	vuint32 ky		 = blend(kx + 1, vuint32(0), kx < 2);

	const vfloat dZ = pick(in.Direction, kz);

	// Swap
	const bfloat sw	  = dZ < 0;
	const vuint32 tmp = blend(kx, ky, sw);
	kx				  = blend(ky, kx, sw);
	ky				  = tmp;

	const vfloat dX = pick(in.Direction, kx);
	const vfloat dY = pick(in.Direction, ky);

	const vfloat sx = dX / dZ;
	const vfloat sy = dY / dZ;
	const vfloat sz = 1.0f / dZ;

	// We use (1-u-v)*P1 + u*P2 + v*P3 convention
	Vector3fv A = p2 - in.Origin;
	Vector3fv B = p3 - in.Origin;
	Vector3fv C = p1 - in.Origin;

	// Shear
	const vfloat Akz = pick(A, kz);
	const vfloat Bkz = pick(B, kz);
	const vfloat Ckz = pick(C, kz);

	const vfloat Ax = pick(A, kx) - sx * Akz;
	const vfloat Ay = pick(A, ky) - sy * Akz;
	const vfloat Bx = pick(B, kx) - sx * Bkz;
	const vfloat By = pick(B, ky) - sy * Bkz;
	const vfloat Cx = pick(C, kx) - sx * Ckz;
	const vfloat Cy = pick(C, ky) - sy * Ckz;

	vfloat u = Cx * By - Cy * Bx;
	vfloat v = Ax * Cy - Ay * Cx;
	vfloat w = Bx * Ay - By * Ax;

	// Better precision is not provided for SIMD...
	bfloat valid = ~((min(min(u, v), w) < -PR_TRIANGLE_WT_INTERSECT_EPSILON)
					 & (max(max(u, v), w) > PR_TRIANGLE_WT_INTERSECT_EPSILON));
	if (none(valid))
		return valid;

	const vfloat det = u + v + w;
	valid			 = valid & (abs(det) > PR_TRIANGLE_WT_INTERSECT_EPSILON);
	if (none(valid))
		return valid;

	const vfloat Az = sz * Akz;
	const vfloat Bz = sz * Bkz;
	const vfloat Cz = sz * Ckz;

	t	  = u * Az + v * Bz + w * Cz;
	valid = valid & (~(signbit(t) ^ signbit(det)));
	if (none(valid))
		return valid;

	const vfloat invDet = 1.0f / det;
	uv[0]				= u * invDet;
	uv[1]				= v * invDet;
	t *= invDet;

	return valid & in.isInsideRange(t);
}
} // namespace TriangleIntersection
} // namespace PR
