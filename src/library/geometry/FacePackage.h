#pragma once

#include "math/SIMD.h"

namespace PR {
class PR_LIB FacePackage {
public:
	vfloat Vx[3];
	vfloat Vy[3];
	vfloat Vz[3];

	// The followings are the normals (not tangents)
	vfloat Nx[3];
	vfloat Ny[3];
	vfloat Nz[3];

	// TODO: Add tangents and bitangents

	vfloat U[3];
	vfloat V[3];

	inline void interpolate(const vfloat& b1, const vfloat& b2,
							vfloat& vx, vfloat& vy, vfloat& vz,
							vfloat& nx, vfloat& ny, vfloat& nz,
							vfloat& u, vfloat& v) const
	{
		const vfloat b3 = (1 - b1 - b2);

		vx = Vx[1] * b1 + Vx[2] * b2 + Vx[0] * b3;
		vy = Vy[1] * b1 + Vy[2] * b2 + Vy[0] * b3;
		vz = Vz[1] * b1 + Vz[2] * b2 + Vz[0] * b3;

		nx = Nx[1] * b1 + Nx[2] * b2 + Nx[0] * b3;
		ny = Ny[1] * b1 + Ny[2] * b2 + Ny[0] * b3;
		nz = Nz[1] * b1 + Nz[2] * b2 + Nz[0] * b3;

		u = U[1] * b1 + U[2] * b2 + U[0] * b3;
		v = V[1] * b1 + V[2] * b2 + V[0] * b3;
	}

	vuint32 MaterialSlot;
};
} // namespace PR
