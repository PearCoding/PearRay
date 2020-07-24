#pragma once

#include "PR_Config.h"

namespace PR {
namespace Spherical {

inline Vector2f from_direction(const Vector3f& D)
{
	float x		= (D(0) == 0 && D(1) == 0) ? 1e-5f : D(0);
	float phi	= atan2(D(1), x);
	phi			= phi < 0 ? phi + 2 * PR_PI : phi; // We want phi to be [0,2pi] not [-pi,pi]
	float theta = acos(D(2));
	return Vector2f(theta, phi);
}

inline Vector2f uv_from_normal(const Vector3f& N)
{
	Vector2f tp = from_direction(N) * PR_1_PI;
	return Vector2f(tp(1) / 2, tp(0));
}

inline Vector2f uv_from_point(const Vector3f& V)
{
	return uv_from_normal(V.normalized());
}

// theta [0, PI]
// phi [0, 2*PI]
inline Vector3f cartesian(float thSin, float thCos, float phSin, float phCos)
{
	return Vector3f(thSin * phCos,
					thSin * phSin,
					thCos);
}

inline Vector3f cartesian(float theta, float phi)
{
	return cartesian(sin(theta), cos(theta),
					 sin(phi), cos(phi));
}

// u,v [0, 1]
inline Vector3f cartesian_from_uv(float u, float v)
{
	return cartesian(v * PR_PI,
					 u * 2 * PR_PI);
}

} // namespace Spherical
} // namespace PR
