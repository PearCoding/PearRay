#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "math/Spherical.h"
#include "renderer/RenderContext.h"
#include "shader/INodePlugin.h"

#include "skymodel/ArHosekSkyModel.h"

#include "DebugIO.h"

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

namespace PR {
constexpr size_t SPECTRAL_BANDS = 11; // The standard (v.1.4) has 11 bins
constexpr float SPECTRAL_DELTA	= 40;
constexpr float SPECTRAL_START	= 320;
constexpr float SPECTRAL_END	= SPECTRAL_START + SPECTRAL_BANDS * SPECTRAL_DELTA;

constexpr size_t RES_PHI   = 512;
constexpr size_t RES_THETA = 512;

// Spherical stuff
struct ElevationAzimuth {
	float Elevation;
	float Azimuth;
};

inline ElevationAzimuth eaFromThetaPhi(float theta, float phi)
{
	auto ea = ElevationAzimuth{ std::asin(std::sin(phi) * std::sin(theta)), std::atan(std::cos(phi) * std::tan(theta)) };
	if (ea.Azimuth < 0)
		ea.Azimuth += 2 * PR_PI;
	return ea;
}
inline ElevationAzimuth eaFromDirection(const Vector3f& direction)
{
	auto ea = ElevationAzimuth{ std::acos(direction(1)), std::atan2(direction(0), -direction(2)) };
	if (ea.Azimuth < 0)
		ea.Azimuth += 2 * PR_PI;
	return ea;
}

// Sun stuff
struct TimePoint {
	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;
	float Seconds;
};

struct MapLocation {
	float Longitude;
	float Latitude;
	float Timezone;
};

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
			rightAscension += 2 * M_PI;
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

class SkyModel {
public:
	SkyModel(const std::shared_ptr<FloatSpectralNode>& ground_albedo, const ElevationAzimuth& sunEA, const ParameterGroup& params)
	{
		mPhiCount	= params.getInt("phi_resolution", RES_PHI);
		mThetaCount = params.getInt("theta_resolution", RES_THETA);

		const float solar_elevation		  = 0.5f * PR_PI - sunEA.Elevation;
		const float atmospheric_turbidity = params.getNumber("turbidity", 3.0f);

		mData.resize(mPhiCount * mThetaCount * SPECTRAL_BANDS);
		for (size_t k = 0; k < SPECTRAL_BANDS; ++k) {
			float wavelength = SPECTRAL_START + k * SPECTRAL_DELTA;
			// Evaluate albedo for particular wavelength -> Does not support textures etc
			ShadingContext ctx;
			ctx.WavelengthNM = SpectralBlob(wavelength);

			const float albedo = ground_albedo->eval(ctx)[0];

			auto* state = arhosekskymodelstate_alloc_init(solar_elevation, atmospheric_turbidity, albedo);
			auto body	= [&](const tbb::blocked_range2d<size_t, size_t>& r) {
				  for (size_t y = r.rows().begin(); y != r.rows().end(); ++y) {
					  const float theta = PR_PI * y / (float)mThetaCount;
					  if (theta >= PR_PI / 2) {
						  for (size_t x = r.cols().begin(); x != r.cols().end(); ++x) {
							  mData[y * mPhiCount * SPECTRAL_BANDS + x * SPECTRAL_BANDS + k] = 0;
						  }
					  } else {
						  for (size_t x = r.cols().begin(); x != r.cols().end(); ++x) {
							  const float phi = 2 * PR_PI * x / (float)mPhiCount;

							  ElevationAzimuth ea = eaFromThetaPhi(theta, phi);
							  float cosGamma	  = std::cos(theta) * std::cos(sunEA.Elevation)
											   + std::sin(theta) * std::sin(sunEA.Elevation) * std::cos(ea.Azimuth - sunEA.Azimuth);
							  float gamma	 = std::acos(std::min(1.0f, std::max(-1.0f, cosGamma)));
							  float radiance = arhosekskymodel_radiance(state, theta, gamma, wavelength + 0.005f /*Make sure the correct bin is choosen*/);

							  mData[y * mPhiCount * SPECTRAL_BANDS + x * SPECTRAL_BANDS + k] = std::max(0.0f, radiance);
						  }
					  }
				  }
			};
			tbb::parallel_for(tbb::blocked_range2d<size_t, size_t>(0, mThetaCount, 0, mPhiCount), body);

			arhosekskymodelstate_free(state);
		}

		Debug::saveImage("sky.exr", mData.data(), mPhiCount, mThetaCount, SPECTRAL_BANDS);
	}

	inline size_t phiCount() const { return mPhiCount; }
	inline size_t thetaCount() const { return mThetaCount; }

	inline float radiance(int wvl_band, float u, float v) const
	{
		int x = std::max(0, std::min<int>(mPhiCount - 1, int(u * mPhiCount)));
		int y = std::max(0, std::min<int>(mThetaCount - 1, int(v * mThetaCount)));
		return mData[y * mPhiCount * SPECTRAL_BANDS + x * SPECTRAL_BANDS + wvl_band];
	}

private:
	std::vector<float> mData;

	size_t mPhiCount;
	size_t mThetaCount;
};

class SkyNode : public FloatSpectralNode {
public:
	explicit SkyNode(const SkyModel& model)
		: mModel(model)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float f	= std::min<float>(SPECTRAL_BANDS - 2, std::max(0.0f, (ctx.WavelengthNM[i] - SPECTRAL_START) / SPECTRAL_DELTA));
			const int index = f;
			const float t	= f - index;

			float radiance = mModel.radiance(index, ctx.UV(1), ctx.UV(0)) * (1 - t) + mModel.radiance(index + 1, ctx.UV(1), ctx.UV(0)) * t;
			blob[i]		   = radiance;
		}
		return blob;
	}

	Vector2i queryRecommendedSize() const override
	{
		return Vector2i(mModel.phiCount(), mModel.thetaCount());
	}
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SkyNode (" << mModel.phiCount() << "x" << mModel.thetaCount() << ")"; // TODO: Better information?
		return sstream.str();
	}

private:
	const SkyModel mModel;
};

class SkySunPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		auto ground_albedo = ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("albedo"), 0.2f);
		ElevationAzimuth sunEA;
		if (ctx.Parameters.hasParameter("direction")) {
			sunEA = eaFromDirection(ctx.Parameters.getVector3f("direction", Vector3f(0, 0, 1)));
		} else {
			TimePoint timepoint;
			MapLocation location;
			timepoint.Year	   = ctx.Parameters.getInt("year", 2020); // Saarbruecken 2020.05.06
			timepoint.Month	   = ctx.Parameters.getInt("month", 5);
			timepoint.Day	   = ctx.Parameters.getInt("day", 6);
			timepoint.Hour	   = ctx.Parameters.getInt("hour", 12);
			timepoint.Minute   = ctx.Parameters.getInt("minute", 0);
			timepoint.Seconds  = ctx.Parameters.getNumber("seconds", 0);
			location.Latitude  = ctx.Parameters.getNumber("latitude", 49.235422f);
			location.Longitude = ctx.Parameters.getNumber("longitude", 6.9965744f);
			location.Timezone  = ctx.Parameters.getNumber("timezone", 2);
			sunEA			   = computeSunEA(timepoint, location);
		}

		PR_LOG(L_INFO) << "Sun: " << sunEA.Elevation << " " << sunEA.Azimuth << std::endl;

		if (type_name == "sky") {
			return std::make_shared<SkyNode>(SkyModel(ground_albedo, sunEA, ctx.Parameters));
		}// TODO: Add sun and composition model!
		return nullptr;
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sky", "sun", "skysun" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SkySunPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)