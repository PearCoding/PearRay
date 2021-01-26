#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "geometry/Disk.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Concentric.h"
#include "math/ImportanceSampling.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "scene/Scene.h"
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
	SunLight(const std::shared_ptr<ServiceObserver>& so,
			 const std::string& name, const Transformf& transform,
			 const ElevationAzimuth& ea, float turbidity, float radius, float scale)
		: IInfiniteLight(name, transform)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection((normalMatrix() * ea.toDirection()).normalized())
		, mCosTheta(std::cos(SUN_VIS_RADIUS * radius))
		, mPDF(Sampling::uniform_cone_pdf(mCosTheta))
		, mSceneRadius(0)
		, mServiceObserver(so)
		, mCBID(0)
	{
		Tangent::frame(mDirection, mDx, mDy);

		scale /= (radius * radius); // Compensate for different radii
		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * scale;

		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius();
			});
	}

	virtual ~SunLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		const float cosine = std::max(0.0f, in.Direction.dot(mDirection));
		//PR_ASSERT(cosine >= 0.0f && cosine <= 1.0f, "cosine must be between 0 and 1");

		if (cosine < mCosTheta) {
			out.Radiance		= SpectralBlob::Zero();
			out.Direction_PDF_S = 0;
		} else {
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out.Radiance[i] = mSpectrum.lookup(in.WavelengthNM[i]);

			out.Direction_PDF_S = mPDF;
		}
	}

	void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out,
				   const RenderTileSession&) const override
	{
		const Vector3f dir	= Sampling::uniform_cone(in.DirectionRND(0), in.DirectionRND(1), mCosTheta);
		out.Outgoing		= Tangent::fromTangentSpace(mDirection, mDx, mDy, dir);
		out.Direction_PDF_S = mPDF;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Radiance[i] = mSpectrum.lookup(in.WavelengthNM[i]);
	}

	void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out,
					  const RenderTileSession& session) const override
	{
		sampleDir(in, out, session);

		if (in.Point) {
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		} else {
			const Vector2f uv  = mSceneRadius * Concentric::square2disc(in.PositionRND);
			out.LightPosition  = mSceneRadius * out.Outgoing + uv(0) * mDx + uv(1) * mDy;
			out.Position_PDF_A = 1 / (2 * PR_PI * mSceneRadius);
		}
	}

	SpectralBlob power(const SpectralBlob& wvl) const override
	{
		SpectralBlob res;
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = mSpectrum.lookup(wvl[i]);
		return res;
	}
	SpectralRange spectralRange() const override { return SpectralRange(mSpectrum.wavelengthStart(), mSpectrum.wavelengthEnd()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunLight>:" << std::endl
			   << "    Elevation: " << PR_RAD2DEG * mEA.Elevation << "°" << std::endl
			   << "    Azimuth:   " << PR_RAD2DEG * mEA.Azimuth << "°" << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	const ElevationAzimuth mEA;
	const Vector3f mDirection;
	Vector3f mDx;
	Vector3f mDy;
	const float mCosTheta;
	const float mPDF;

	float mSceneRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class SunDeltaLight : public IInfiniteLight {
public:
	SunDeltaLight(const std::shared_ptr<ServiceObserver>& so,
				  const std::string& name, const Transformf& transform,
				  const ElevationAzimuth& ea, float turbidity, float scale)
		: IInfiniteLight(name, transform)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection((normalMatrix() * ea.toDirection()).normalized())
		, mSceneRadius(0)
		, mServiceObserver(so)
		, mCBID(0)
	{
		Tangent::frame(mDirection, mDx, mDy);

		const float solid_angle = 2 * PR_PI * (1 - std::cos(SUN_VIS_RADIUS));
		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * solid_angle * scale;

		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius();
			});
	}

	virtual ~SunDeltaLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	bool hasDeltaDistribution() const override { return true; }
	void eval(const InfiniteLightEvalInput&, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_ASSERT(false, "eval() for delta lights should never be called!");

		out.Radiance		= SpectralBlob::Zero();
		out.Direction_PDF_S = 0;
	}

	void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out,
				   const RenderTileSession&) const override
	{
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Radiance[i] = mSpectrum.lookup(in.WavelengthNM[i]);

		out.Outgoing		= mDirection;
		out.Direction_PDF_S = 1;
	}

	void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out,
					  const RenderTileSession& session) const override
	{
		sampleDir(in, out, session);

		if (in.Point) {
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		} else {
			const Vector2f uv  = mSceneRadius * Concentric::square2disc(in.PositionRND);
			out.LightPosition  = mSceneRadius * out.Outgoing + uv(0) * mDx + uv(1) * mDy;
			out.Position_PDF_A = 1 / (2 * PR_PI * mSceneRadius);
		}
	}

	SpectralBlob power(const SpectralBlob& wvl) const override
	{
		SpectralBlob res;
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = mSpectrum.lookup(wvl[i]);
		return res;
	}
	SpectralRange spectralRange() const override { return SpectralRange(mSpectrum.wavelengthStart(), mSpectrum.wavelengthEnd()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunDeltaLight>:" << std::endl
			   << "    Elevation: " << PR_RAD2DEG * mEA.Elevation << "°" << std::endl
			   << "    Azimuth:   " << PR_RAD2DEG * mEA.Azimuth << "°" << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	const ElevationAzimuth mEA;
	const Vector3f mDirection;
	Vector3f mDx;
	Vector3f mDy;

	float mSceneRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class SunLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");

		ElevationAzimuth sunEA = computeSunEA(ctx.parameters());
		const float radius	   = ctx.parameters().getNumber("radius", 1.0f);
		const float turbidity  = ctx.parameters().getNumber("turbidity", 3.0f);
		const float scale	   = ctx.parameters().getNumber("power_scale", 1.0f);

		const std::shared_ptr<ServiceObserver> so = ctx.environment()->serviceObserver();

		if (radius <= PR_EPSILON)
			return std::make_shared<SunDeltaLight>(so, name, ctx.transform(), sunEA, turbidity, scale);
		else
			return std::make_shared<SunLight>(so, name, ctx.transform(), sunEA,
											  turbidity, radius, scale);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sun" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		//TODO: Make sun location a a function to be shared with sky.cpp
		TimePoint timepoint;
		MapLocation location;
		return PluginSpecificationBuilder("Sun", "Simple sun model")
			.Identifiers(getNames())
			.Inputs()
			.Number("radius", "Radius of sun in respect to image space", 1.0f)
			.Number("turbidity", "Sky turbidity", 3.0f)
			.Number("power_scale", "Amount of scale applied to the power of the sun", 1.0f)
			.BeginBlock("Location", PluginParamDescBlockOp::OneOf)
			.BeginBlock("")
			.Vector("direction", "Direction the light is coming from", Vector3f(0, 0, 1))
			.EndBlock()
			.BeginBlock("")
			.Number("theta", "Sun theta in radians", 0.0f)
			.Number("phi", "Sun phi in radians", 0.0f)
			.EndBlock()
			.BeginBlock("")
			.Number("elevation", "Sun elevation in radians", 0.0f)
			.Number("azimuth", "Sun azimuth in radians", 0.0f)
			.EndBlock()
			.BeginBlock("")
			.UInt("year", "Year", timepoint.Year)
			.UInt("month", "Month", timepoint.Month)
			.UInt("day", "Day", timepoint.Day)
			.UInt("hour", "Hour", timepoint.Hour)
			.UInt("minute", "Minute", timepoint.Minute)
			.Number("seconds", "Seconds", timepoint.Seconds)
			.Number("latitude", "Latitude", location.Latitude)
			.Number("longitude", "Longitude", location.Longitude)
			.Number("timezone", "Timezone", location.Timezone)
			.EndBlock()
			.EndBlock()
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SunLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)