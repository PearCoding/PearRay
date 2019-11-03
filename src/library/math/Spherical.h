#pragma once

#include "PR_Config.h"

namespace PR {
namespace Spherical {

template <typename T>
inline Vector2t<T> uv_from_normal(const Vector3t<T>& N)
{
	T u = acos(N(2)) * PR_1_PI;
	T v = (T(1) + atan2(N(1), N(0)) * PR_1_PI) * 0.5f;
	return Vector2t<T>(u, v);
}

template <typename T>
inline Vector2t<T> uv_from_point(const Vector3t<T>& V)
{
	return uv_from_normal<T>(V / V.norm());
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
	return cartesian(u * PR_PI, (2 * v - T(1)) * PR_PI);
}

} // namespace Spherical
} // namespace PR
