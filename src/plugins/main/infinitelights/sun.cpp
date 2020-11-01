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

inline float calculatePosDiskRadius(float scene_radius, float cosTheta)
{
	return scene_radius * std::sqrt((1 - cosTheta) * (1 + cosTheta));
}

class SunLight : public IInfiniteLight {
public:
	SunLight(const std::shared_ptr<ServiceObserver>& so, 
			 uint32 id, const std::string& name, const ElevationAzimuth& ea, float turbidity, float radius, float scale,
			 const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection(trans * normalMatrix() * ea.toDirection().normalized())
		, mCosTheta(std::cos(SUN_VIS_RADIUS * radius))
		, mPDF(Sampling::uniform_cone_pdf(mCosTheta))
		, mSceneRadius(0)
		, mPosDiskRadius(0)
		, mServiceObserver(so)
	{
		Tangent::frame(mDirection, mDx, mDy);

		scale /= (radius * radius); // Compensate for different radii
		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * scale;
						   
		if(mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene){
				mSceneRadius   = scene->boundingSphere().radius();
				mPosDiskRadius = calculatePosDiskRadius(mSceneRadius, std::abs(mDirection(2)));
			});
	}

	virtual ~SunLight() {
		if(mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		const float cosine = std::max(0.0f, in.Ray.Direction.dot(mDirection));
		//PR_ASSERT(cosine >= 0.0f && cosine <= 1.0f, "cosine must be between 0 and 1");

		if (cosine < mCosTheta) {
			out.Radiance = SpectralBlob::Zero();
			out.PDF_S	 = 0;
		} else {
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out.Radiance[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);

			out.PDF_S = mPDF;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		const Vector3f dir = Sampling::uniform_cone(in.RND(0), in.RND(1), mCosTheta);
		out.Outgoing	   = Tangent::fromTangentSpace(mDirection, mDx, mDy, dir);
		out.PDF_S		   = mPDF;

		if (in.Point) {
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		} else if (in.SamplePosition) {
			const Vector2f uv = mPosDiskRadius * Concentric::square2disc(Vector2f(in.RND(0), in.RND(1)));
			out.LightPosition = mSceneRadius * out.Outgoing + uv(0) * mDx + uv(1) * mDy;
			// TODO: PDF?
		}

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Radiance[i] = mSpectrum.lookup(in.WavelengthNM[i]);
	}

	float power() const override { return mSpectrum.average(); }

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
	float mPosDiskRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class SunDeltaLight : public IInfiniteLight {
public:
	SunDeltaLight(const std::shared_ptr<ServiceObserver>& so, 
			 	  uint32 id, const std::string& name, const ElevationAzimuth& ea, float turbidity, float scale,
				  const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mEA(ea)
		, mDirection(trans * normalMatrix() * ea.toDirection().normalized())
		, mSceneRadius(0)
		, mPosDiskRadius(0)
		, mServiceObserver(so)
	{
		Tangent::frame(mDirection, mDx, mDy);

		const float solid_angle = 2 * PR_PI * (1 - std::cos(SUN_VIS_RADIUS));
		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(mSpectrum.wavelengthStart() + i * mSpectrum.delta(),
											  mEA.theta(), turbidity)
						   * solid_angle * scale;
			
		if(mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene){
				mSceneRadius   = scene->boundingSphere().radius();
				mPosDiskRadius = calculatePosDiskRadius(mSceneRadius, std::abs(mDirection(2)));
			});
	}

	virtual ~SunDeltaLight() {
		if(mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	bool hasDeltaDistribution() const override { return true; }
	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_ASSERT(false, "eval() for delta lights should never be called!");

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Radiance[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);
		out.PDF_S = PR_INF;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Radiance[i] = mSpectrum.lookup(in.WavelengthNM[i]);

		out.Outgoing = mDirection;
		out.PDF_S	 = PR_INF;

		if (in.Point) {
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		} else if (in.SamplePosition) {
			const Vector2f uv = mPosDiskRadius * Concentric::square2disc(Vector2f(in.RND(0), in.RND(1)));
			out.LightPosition = mSceneRadius * out.Outgoing + uv(0) * mDx + uv(1) * mDy;
			// TODO: PDF?
		}
	}

	float power() const override { return mSpectrum.average(); }

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
	float mPosDiskRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class SunLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");
		Eigen::Matrix3f trans  = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		ElevationAzimuth sunEA = computeSunEA(ctx.parameters());
		const float radius	   = ctx.parameters().getNumber("radius", 1.0f);
		const float turbidity  = ctx.parameters().getNumber("turbidity", 3.0f);
		const float scale	   = ctx.parameters().getNumber("scale", 1.0f);

		const std::shared_ptr<ServiceObserver> so = ctx.hasEnvironment() ? ctx.environment()->serviceObserver() : nullptr;

		if (radius <= PR_EPSILON)
			return std::make_shared<SunDeltaLight>(so, id, name, sunEA, turbidity, scale, trans);
		else
			return std::make_shared<SunLight>(so, id, name, sunEA,
											  turbidity, radius, scale, trans);
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