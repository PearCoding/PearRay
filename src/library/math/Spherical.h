#pragma once

#include "PR_Config.h"

namespace PR {
namespace Spherical {

inline Vector2f uv_from_normal(const Vector3f& N)
{
	float x = (N(0) == 0 && N(1) == 0) ? 1e-5f : N(0);
	float u = atan2(N(1), x) * PR_1_PI;
	u		= u < 0 ? u + 2 : u;
	u /= 2;

	float v = acos(N(2)) * PR_1_PI;
	return Vector2f(u, v);
}

inline Vector2fv uv_from_normal(const Vector3fv& N)
{
	vfloat u = atan2(N(1), N(0)) * PR_1_PI;
	u		 = blend(u + 2, u, u < 0) / 2;

	vfloat v = acos(N(2)) * PR_1_PI;
	return Vector2fv(u, v);
}

template <typename T>
inline Vector2t<T> uv_from_point(const Vector3t<T>& V)
{
	return uv_from_normal(Vector3t<T>(V / V.norm()));
}

// theta [0, PI]
// phi [0, 2*PI]
template <typename T>
inline Vector3t<T> cartesian(const T& thSin, const T& thCos, const T& phSin, const T& phCos)
{
	return Vector3t<T>(thSin * phCos,
					   thSin * phSin,
					   thCos);
}

template <typename T>
inline Vector3t<T> cartesian(const T& theta, const T& phi)
{
	return cartesian(sin(theta), cos(theta),
					 sin(phi), cos(phi));
}

// u,v [0, 1]
template <typename T>
inline Vector3t<T> cartesian_from_uv(const T& u, const T& v)
{
	return cartesian(v * PR_PI,
					 u * 2 * PR_PI);
}

} // namespace Spherical
} // namespace PR
