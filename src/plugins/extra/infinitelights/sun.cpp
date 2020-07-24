#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "geometry/Disk.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/ImportanceSampling.h"
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
		, mDirection(dir.normalized())
		, mDisk(PR_DEG2RAD * SUN_VIS_RADIUS * 0.5f * radius) // TODO: Radius should not be incalculated here
		, mCosTheta(std::abs(std::cos(mDisk.radius())))
		, mSolidAngle(2 * PR_PI * (1 - std::cos(mDisk.radius())))
	{
		Tangent::frame(mDirection, mDx, mDy);

		const float theta = mDisk.radius();

		for (size_t i = 0; i < mSpectrum.sampleCount(); ++i)
			mSpectrum[i] = computeSunRadiance(SUN_WAVELENGTH_START + (SUN_WAVELENGTH_END - SUN_WAVELENGTH_START) * i / (SUN_WAVELENGTH_SAMPLES - 1.0f),
											  theta, turbidity)
						   * mSolidAngle;
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		const float cosine = std::max(0.0f, in.Ray.Direction.dot(mDirection));
		if (cosine < mCosTheta) {
			out.Weight = SpectralBlob::Zero();
			out.PDF_S  = 0;
		} else {
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out.Weight[i] = mSpectrum.lookup(in.Ray.WavelengthNM[i]);

			out.PDF_S = 1 / mSolidAngle;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		out.Outgoing	   = Tangent::fromTangentSpace(mDirection, mDx, mDy,
												   (mDisk.surfacePoint(in.RND(0), in.RND(1)) + Vector3f(0, 0, 1)).normalized());
		//const float cosine = std::abs(out.Outgoing.dot(mDirection));

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.Weight[i] = mSpectrum.lookup(in.Point.Ray.WavelengthNM[i]);

		out.PDF_S = 1 / mSolidAngle;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SunLight>:" << std::endl
			   << "    Direction: " << PR_FMT_MAT(mDirection) << std::endl
			   << "    Radius: " << mDisk.radius() << std::endl;
		return stream.str();
	}

private:
	EquidistantSpectrum mSpectrum;
	Vector3f mDirection;
	Vector3f mDx;
	Vector3f mDy;
	Disk mDisk;
	float mCosTheta;
	float mSolidAngle;
};

class SunLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");

		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		PR_LOG(L_INFO) << "Sun: " << sunEA.Elevation << " " << sunEA.Azimuth << std::endl;
		const Vector3f dir = sunEA.toDirection();

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