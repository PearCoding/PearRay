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
