#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Projection.h"
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
constexpr float SUN_VIS_RADIUS			= 0.5358f;

class SunLight : public IInfiniteLight {
public:
	SunLight(uint32 id, const std::string& name, const Vector3f& dir, float turbidity, float radius)
		: IInfiniteLight(id, name)
		, mSpectrum(SUN_WAVELENGTH_SAMPLES, SUN_WAVELENGTH_START, SUN_WAVELENGTH_END)
		, mDirection(dir)
		, mRadius(radius)
	{
		const float theta = PR_DEG2RAD * SUN_VIS_RADIUS * 0.5f;
		//const float cosTheta = std::cos(theta * radius);
		const float factor = 2 * PR_PI * (1 - std::cos(theta));

		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(SUN_WAVELENGTH_START + (SUN_WAVELENGTH_END - SUN_WAVELENGTH_START) * i / (SUN_WAVELENGTH_SAMPLES - 1.0f),
											  theta, turbidity)
						   * factor;
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		// TODO: Eval based on disk!

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		// TODO: Sample based on disk!

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Point.Ray.WavelengthNM[i]);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunLight>:" << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl
			   << "    Radius: " << mRadius << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	Vector3f mDirection;
	float mRadius;
};

class SunLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");

		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		PR_LOG(L_INFO) << "Sun: " << sunEA.Elevation << " " << sunEA.Azimuth << std::endl;
		const Vector3f dir = Vector3f(0,0,1);// TODO: From sunEA!

		return std::make_shared<SunLight>(id, name, dir, ctx.Parameters.getNumber("turbidity", 3.0f), ctx.Parameters.getNumber("radius", 1.0f));
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