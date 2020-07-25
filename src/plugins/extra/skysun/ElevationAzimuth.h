#pragma once

#include "PR_Config.h"

namespace PR {
constexpr float ELEVATION_RANGE = PR_PI * 0.5f;
constexpr float AZIMUTH_RANGE	= PR_PI * 2;
// Up is Y+
struct ElevationAzimuth {
	float Elevation; // [0, pi/2]
	float Azimuth;	 // [0, 2pi]

	// Based on https://www.mathworks.com/help/phased/ref/azel2phitheta.html (alternate theta/phi definition)
	inline static ElevationAzimuth fromThetaPhi(float theta, float phi)
	{
		return ElevationAzimuth{ 0.5f * PR_PI - theta, phi };
	}

	inline static ElevationAzimuth fromDirection(const Vector3f& direction)
	{
		auto ea = ElevationAzimuth{ std::atan2(direction(1), std::sqrt(direction(0) * direction(0) + direction(2) * direction(2))),
									std::atan2(direction(2), direction(0)) };
		if (ea.Azimuth < 0)
			ea.Azimuth += 2 * PR_PI;
		return ea;
	}

	inline Vector3f toDirection() const
	{
		float sinEl = std::sin(Elevation);
		float cosEl = std::cos(Elevation);
		float sinAz = std::sin(Azimuth);
		float cosAz = std::cos(Azimuth);

		return Vector3f(cosAz * cosEl, sinEl, sinAz * cosEl);
	}

	inline Vector2f toThetaPhi() const
	{
		return Vector2f(0.5f * PR_PI - Elevation, Azimuth);
	}
};
} // namespace PR