#include "SunLocation.h"
#include "SceneLoadContext.h"

namespace PR {

/* Based on "Computing the Solar Vector" by Manuel Blanco-Muriel,
 * Diego C. Alarcon-Padilla, Teodoro Lopez-Moratalla, and Martin Lara-Coira,
 * in "Solar energy", vol 27, number 5, 2001 by Pergamon Press.
 */
ElevationAzimuth computeSunEA(const TimePoint& timepoint, const MapLocation& location)
{
	constexpr double EARTH_MEAN_RADIUS = 6371.01;	// In km
	constexpr double ASTRONOMICAL_UNIT = 149597890; // In km

	// Auxiliary variables
	double dY;
	double dX;

	/* Calculate difference in days between the current Julian Day
       and JD 2451545.0, which is noon 1 January 2000 Universal Time */
	double elapsedJulianDays, decHours;
	{
		// Calculate time of the day in UT decimal hours
		decHours = timepoint.Hour - location.Timezone + (timepoint.Minute + timepoint.Seconds / 60.0) / 60.0;

		// Calculate current Julian Day
		int liAux1 = (timepoint.Month - 14) / 12;
		int liAux2 = (1461 * (timepoint.Year + 4800 + liAux1)) / 4
					 + (367 * (timepoint.Month - 2 - 12 * liAux1)) / 12
					 - (3 * ((timepoint.Year + 4900 + liAux1) / 100)) / 4
					 + timepoint.Day - 32075;
		double dJulianDate = (double)liAux2 - 0.5 + decHours / 24.0;

		// Calculate difference between current Julian Day and JD 2451545.0
		elapsedJulianDays = dJulianDate - 2451545.0;
	}

	/* Calculate ecliptic coordinates (ecliptic longitude and obliquity of the
       ecliptic in radians but without limiting the angle to be less than 2*Pi
       (i.e., the result may be greater than 2*Pi) */
	double eclipticLongitude, eclipticObliquity;
	{
		double omega		 = 2.1429 - 0.0010394594 * elapsedJulianDays;
		double meanLongitude = 4.8950630 + 0.017202791698 * elapsedJulianDays; // Radians
		double anomaly		 = 6.2400600 + 0.0172019699 * elapsedJulianDays;

		eclipticLongitude = meanLongitude + 0.03341607 * std::sin(anomaly)
							+ 0.00034894 * std::sin(2 * anomaly) - 0.0001134
							- 0.0000203 * std::sin(omega);

		eclipticObliquity = 0.4090928 - 6.2140e-9 * elapsedJulianDays
							+ 0.0000396 * std::cos(omega);
	}

	/* Calculate celestial coordinates ( right ascension and declination ) in radians
       but without limiting the angle to be less than 2*Pi (i.e., the result may be
       greater than 2*Pi) */
	double rightAscension, declination;
	{
		double sinEclipticLongitude = std::sin(eclipticLongitude);
		dY							= std::cos(eclipticObliquity) * sinEclipticLongitude;
		dX							= std::cos(eclipticLongitude);
		rightAscension				= std::atan2(dY, dX);
		if (rightAscension < 0.0)
			rightAscension += 2 * PR_PI;
		declination = std::asin(std::sin(eclipticObliquity) * sinEclipticLongitude);
	}

	// Calculate local coordinates (azimuth and zenith angle) in degrees
	double elevation, azimuth;
	{
		double greenwichMeanSiderealTime = 6.6974243242
										   + 0.0657098283 * elapsedJulianDays + decHours;

		double localMeanSiderealTime = PR_DEG2RAD * ((float)((greenwichMeanSiderealTime * 15 + location.Longitude)));

		double latitudeInRadians = PR_DEG2RAD * location.Latitude;
		double cosLatitude		 = std::cos(latitudeInRadians);
		double sinLatitude		 = std::sin(latitudeInRadians);

		double hourAngle	= localMeanSiderealTime - rightAscension;
		double cosHourAngle = std::cos(hourAngle);

		elevation = std::acos(cosLatitude * cosHourAngle
								  * std::cos(declination)
							  + std::sin(declination) * sinLatitude);

		dY = -std::sin(hourAngle);
		dX = std::tan(declination) * cosLatitude - sinLatitude * cosHourAngle;

		azimuth = std::atan2(dY, dX);
		if (azimuth < 0.0)
			azimuth += 2 * PR_PI;

		// Parallax Correction
		elevation += (EARTH_MEAN_RADIUS / ASTRONOMICAL_UNIT) * std::sin(elevation);
	}

	return ElevationAzimuth{ (float)elevation, (float)azimuth };
}

ElevationAzimuth computeSunEA(const ParameterGroup& params)
{
	if (params.hasParameter("direction")) {
		return ElevationAzimuth::fromDirection(params.getVector3f("direction", Vector3f(0, 0, 1)));
	} else if (params.hasParameter("theta")) {
		return ElevationAzimuth::fromThetaPhi(params.getNumber("theta", 0), params.getNumber("phi", 0));
	} else if (params.hasParameter("elevation")) {
		return ElevationAzimuth{ params.getNumber("elevation", 0), params.getNumber("azimuth", 0) };
	} else {
		TimePoint timepoint;
		MapLocation location;
		timepoint.Year	   = params.getInt("year", timepoint.Year);
		timepoint.Month	   = params.getInt("month", timepoint.Month);
		timepoint.Day	   = params.getInt("day", timepoint.Day);
		timepoint.Hour	   = params.getInt("hour", timepoint.Hour);
		timepoint.Minute   = params.getInt("minute", timepoint.Minute);
		timepoint.Seconds  = params.getNumber("seconds", timepoint.Seconds);
		location.Latitude  = params.getNumber("latitude", location.Latitude);
		location.Longitude = params.getNumber("longitude", location.Longitude);
		location.Timezone  = params.getNumber("timezone", location.Timezone);
		return computeSunEA(timepoint, location);
	}
}
} // namespace PR