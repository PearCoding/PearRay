#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "geometry/Disk.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/ImportanceSampling.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "shader/ShadingContext.h"
#include "spectral/EquidistantSpectrum.h"

#include "skysun/SunLocation.h"
#include "skysun/SunRadiance.h"

namespace PR {
constexpr float SUN_WAVELENGTH_START	= 360.0f;
constexpr float SUN_WAVELENGTH_END		= 760.0f;
constexpr size_t SUN_WAVELENGTH_SAMPLES = 64;
constexpr float SUN_VIS_RADIUS			= PR_DEG2RAD * 0.5358f * 0.5f; // Given in angular radius

class SunLight : public IInfiniteLight {
public:
	SunLight(uint32 id, const std::string& name, const ElevationAzimuth& ea, float turbidity, float radius, float scale)
		: IInfiniteLight(id, name)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection(ea.toDirection().normalized())
		, mRadius(std::abs(std::tan(SUN_VIS_RADIUS)) * radius)
		, mCosTheta(1 / std::sqrt(1 + mRadius * mRadius))
		, mSolidAngle(2 * PR_PI * (1 - mCosTheta))
		, mPDF(1 / mSolidAngle)
	{
		Tangent::frame(mDirection, mDx, mDy);

		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * scale;
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		const float cosine = std::max(0.0f, in.Ray.Direction.dot(mDirection));
		if (cosine < mCosTheta) {
			out.Weight = SpectralBlob::Zero();
			out.PDF_S  = 0;
		} else {
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out.Weight[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);

			out.PDF_S = mPDF;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		const Vector2f uv = mRadius * Sampling::concentric_disk(in.RND(0), in.RND(1));
		out.Outgoing	  = (mDirection + mDx * uv(0) + mDy * uv(1)).normalized();
		out.PDF_S		  = mPDF;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Point.Ray.WavelengthNM[i]);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunLight>:" << std::endl
			   << "    Elevation: " << PR_RAD2DEG * mEA.Elevation << "°" << std::endl
			   << "    Azimuth:   " << PR_RAD2DEG * mEA.Azimuth << "°" << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl
			   << "    Radius:    " << mRadius << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	const ElevationAzimuth mEA;
	const Vector3f mDirection;
	Vector3f mDx;
	Vector3f mDy;
	const float mRadius;
	const float mCosTheta;
	const float mSolidAngle;
	const float mPDF;
};

class SunDeltaLight : public IInfiniteLight {
public:
	SunDeltaLight(uint32 id, const std::string& name, const ElevationAzimuth& ea, float turbidity, float scale)
		: IInfiniteLight(id, name)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection(ea.toDirection().normalized())
		, mSolidAngle(2 * PR_PI * (1 - std::cos(SUN_VIS_RADIUS)))
	{
		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * mSolidAngle * scale;
	}

	bool hasDeltaDistribution() const override { return true; }
	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);
		out.PDF_S = std::numeric_limits<float>::infinity();
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Point.Ray.WavelengthNM[i]);
		out.Outgoing = mDirection;
		out.PDF_S	 = std::numeric_limits<float>::infinity();
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunDeltaLight>:" << std::endl
			   << "    Elevation: " << mEA.Elevation << std::endl
			   << "    Azimuth:   " << mEA.Azimuth << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	const ElevationAzimuth mEA;
	const Vector3f mDirection;
	const float mSolidAngle;
};

class SunLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");

		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		const float radius	   = ctx.Parameters.getNumber("radius", 1.0f);

		if (radius <= PR_EPSILON)
			return std::make_shared<SunDeltaLight>(id, name, sunEA,
												   ctx.Parameters.getNumber("turbidity", 3.0f), ctx.Parameters.getNumber("scale", 1.0f));
		else
			return std::make_shared<SunLight>(id, name, sunEA,
											  ctx.Parameters.getNumber("turbidity", 3.0f), radius,
											  ctx.Parameters.getNumber("scale", 1.0f));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sun" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SunLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)