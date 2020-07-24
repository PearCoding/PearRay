#pragma once

#include "PR_Config.h"

namespace PR {
struct ElevationAzimuth {
	float Elevation;
	float Azimuth;

	inline static ElevationAzimuth fromThetaPhi(float theta, float phi)
	{
		auto ea = ElevationAzimuth{ std::asin(std::sin(phi) * std::sin(theta)), std::atan(std::cos(phi) * std::tan(theta)) };
		if (ea.Azimuth < 0)
			ea.Azimuth += 2 * PR_PI;
		return ea;
	}
	inline static ElevationAzimuth fromDirection(const Vector3f& direction)
	{
		auto ea = ElevationAzimuth{ std::acos(direction(1)), std::atan2(direction(0), -direction(2)) };
		if (ea.Azimuth < 0)
			ea.Azimuth += 2 * PR_PI;
		return ea;
	}
};

// Default is Saarbruecken 2020.05.06 12:00:00 (midday)
struct TimePoint {
	int Year	  = 2020;
	int Month	  = 5;
	int Day		  = 6;
	int Hour	  = 12;
	int Minute	  = 0;
	float Seconds = 0.0f;
};

struct MapLocation {
	float Longitude = 6.9965744f;
	float Latitude	= 49.235422f;
	float Timezone	= 2;
};

ElevationAzimuth computeSunEA(const TimePoint& timepoint, const MapLocation& location);
class ParameterGroup;
ElevationAzimuth computeSunEA(const ParameterGroup& params);
} // namespace PR