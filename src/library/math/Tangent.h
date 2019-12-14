#pragma once

#include "PR_Config.h"

namespace PR {
namespace Tangent {

// Functions applying to the TBN matrix
template <typename T>
inline Vector3t<T> fromTangentSpace(const Vector3t<T>& N,
									const Vector3t<T>& Nx, const Vector3t<T>& Ny,
									const Vector3t<T>& V)
{
	return N * V(2) + Ny * V(1) + Nx * V(0);
}

template <typename T>
inline Vector3t<T> toTangentSpace(const Vector3t<T>& N,
								  const Vector3t<T>& Nx, const Vector3t<T>& Ny,
								  const Vector3t<T>& V)
{
	return Vector3t<T>(Nx(2), Ny(2), N(2)) * V(2) + Vector3t<T>(Nx(1), Ny(1), N(1)) * V(1) + Vector3t<T>(Nx(0), Ny(0), N(0)) * V(0);
}

#define PR_TANGENT_EPS (0.9999f)
inline Vector3f orthogonal_tangent(const Vector3f& N)
{
	if (N(2) < -PR_TANGENT_EPS)
		return Vector3f(1, 0, 0);
	else if (N(2) < PR_TANGENT_EPS)
		return (Eigen::Quaternionf::FromTwoVectors(Vector3f(0, 0, 1), N) * Vector3f(1, 0, 0)).normalized();
	else
		return Vector3f(1, 0, 0);
}

// N Orientation Z+
inline void frame(const Vector3f& N, Vector3f& Nx, Vector3f& Ny)
{
	Nx = orthogonal_tangent(N);
	Ny = N.cross(Nx).normalized();
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

inline void invert_frame(Vector3f& N, Vector3f& Nx, Vector3f& /*Ny*/)
{
	N  = -N;
	Nx = -Nx;
	//Ny = Ny;
}

// Align v on N
template <typename T>
inline Vector3t<T> align(const Vector3t<T>& N, const Vector3t<T>& V)
{
	Vector3t<T> nx, ny;
	frame(N, nx, ny);
	return fromTangentSpace(N, nx, ny, V);
}

} // namespace Tangent
} // namespace PR
