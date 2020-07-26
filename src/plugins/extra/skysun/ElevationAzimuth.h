#pragma once

#include "math/Spherical.h"

namespace PR {
constexpr float ELEVATION_RANGE = PR_PI * 0.5f;
constexpr float AZIMUTH_RANGE	= PR_PI * 2;
// Up is Z+
struct ElevationAzimuth {
	float Elevation; // [0, pi/2]
	float Azimuth;	 // [0, 2pi]

	inline float theta() const { return 0.5f * PR_PI - Elevation; }
	inline float phi() const { return Azimuth; }

	// Based on https://www.mathworks.com/help/phased/ref/azel2phitheta.html (alternate theta/phi definition)
	inline static ElevationAzimuth fromThetaPhi(float theta, float phi)
	{
		auto ea = ElevationAzimuth{ 0.5f * PR_PI - theta, phi };
		if (ea.Azimuth < 0)
			ea.Azimuth += 2 * PR_PI;
		return ea;
	}

	inline static ElevationAzimuth fromDirection(const Vector3f& direction)
	{
		const Vector2f tp = Spherical::from_direction(direction);
		return fromThetaPhi(tp(0), tp(1));
	}

	inline Vector3f toDirection() const
	{
		return Spherical::cartesian(theta(), phi());
	}

	inline Vector2f toThetaPhi() const
	{
		return Vector2f(theta(), phi());
	}
};
} // namespace PR