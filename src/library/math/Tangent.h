#pragma once

#include "PR_Config.h"

namespace PR {
namespace Tangent {

#define PR_TANGENT_EPS (0.9999f)
// N Orientation Z+
inline void frame(const Vector3f& N, Vector3f& Nx, Vector3f& Ny)
{
	const Vector3f t = abs(N(0)) > PR_TANGENT_EPS ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0);
	Nx				 = N.cross(t).normalized();
	Ny				 = N.cross(Nx).normalized();
}

inline void frame(const Vector3fv& N, Vector3fv& Nx, Vector3fv& Ny)
{
	bfloat mask = abs(N(0)) > vfloat(PR_TANGENT_EPS);

	const Vector3fv t = Vector3fv(blend(vfloat(1), vfloat(0), mask),
								  blend(vfloat(0), vfloat(1), mask),
								  vfloat(0));

	Nx = N.cross(t);
	Nx /= Nx.norm();
	Ny = N.cross(Nx);
	Ny /= Ny.norm();
}

template <typename T>
inline Vector3t<T> align(const Vector3t<T>& N,
						 const Vector3t<T>& Nx, const Vector3t<T>& Ny,
						 const Vector3t<T>& V)
{
	return N * V(2) + Ny * V(1) + Nx * V(0);
}

// Align v on N
template <typename T>
inline Vector3t<T> align(const Vector3t<T>& N, const Vector3t<T>& V)
{
	Vector3t<T> nx, ny;
	frame(N, nx, ny);
	return align(N, nx, ny, V);
}
} // namespace Tangent
} // namespace PR
