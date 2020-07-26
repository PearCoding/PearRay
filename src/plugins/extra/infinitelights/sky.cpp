#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Projection.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "shader/ShadingContext.h"

#include "sampler/Distribution2D.h"
#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
class SkyLight : public IInfiniteLight {
public:
	SkyLight(uint32 id, const std::string& name, const SkyModel& model, const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mDistribution()
		, mModel(model)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
	{
		buildDistribution();
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		auto ea = ElevationAzimuth::fromDirection(mInvTransform * in.Ray.Direction);
		if (ea.Elevation < 0) {
			out.Weight = SpectralBlob::Zero();
			out.PDF_S  = 0;
		} else {
			out.Weight		  = radiance(in.Ray.WavelengthNM, ea);
			const float f	  = std::cos(ea.Elevation);
			const float denom = 2 * PR_PI * PR_PI * f;
			out.PDF_S		  = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		Vector2f uv			= mDistribution->sampleContinuous(in.RND, out.PDF_S);
		ElevationAzimuth ea = ElevationAzimuth{ ELEVATION_RANGE * uv(1), AZIMUTH_RANGE * uv(0) };
		out.Outgoing		= mTransform * ea.toDirection();
		const float f		= std::cos(ea.Elevation);
		const float denom	= 2 * PR_PI * PR_PI * f;
		out.PDF_S			= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;

		out.Outgoing = Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);
		out.Weight	 = radiance(in.Point.Ray.WavelengthNM, ea);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SkyLight>:" << std::endl
			   << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		// TODO
		return stream.str();
	}

private:
	inline void buildDistribution()
	{
		mDistribution = std::make_shared<Distribution2D>(mModel.azimuthCount(), mModel.elevationCount());

		const Vector2f filterSize(1.0f / mModel.azimuthCount(), 1.0f / mModel.elevationCount());

		PR_LOG(L_INFO) << "Generating 2d environment (" << mDistribution->width() << "x" << mDistribution->height() << ") of " << name() << std::endl;

		const SpectralBlob WVLS = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test
		mDistribution->generate([&](size_t x, size_t y) {
			const float azimuth	  = AZIMUTH_RANGE * x / (float)(mModel.azimuthCount() - 1);
			const float elevation = ELEVATION_RANGE * y / (float)(mModel.elevationCount() - 1);

			const float f = std::cos(elevation);

			const float val = f * radiance(WVLS, ElevationAzimuth{ elevation, azimuth }).maxCoeff();
			return (val <= PR_EPSILON) ? 0.0f : val;
		});
	}

	inline SpectralBlob radiance(const SpectralBlob& wvls, const ElevationAzimuth& ea) const
	{
		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float f	= std::min<float>(AR_SPECTRAL_BANDS - 2, std::max(0.0f, (wvls[i] - AR_SPECTRAL_START) / AR_SPECTRAL_DELTA));
			const int index = f;
			const float t	= f - index;

			float radiance = mModel.radiance(index, ea) * (1 - t) + mModel.radiance(index + 1, ea) * t;
			blob[i]		   = radiance;
		}
		return blob;
	}

	std::shared_ptr<Distribution2D> mDistribution;
	const SkyModel mModel;
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;
};

class SkyLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");
		Eigen::Matrix3f trans  = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		auto ground_albedo	   = ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("albedo"), 0.2f);
		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		PR_LOG(L_INFO) << "Sun: " << PR_RAD2DEG * sunEA.Elevation << "° " << PR_RAD2DEG * sunEA.Azimuth << "°" << std::endl;

		return std::make_shared<SkyLight>(id, name, SkyModel(ground_albedo, sunEA, ctx.Parameters), trans);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sky" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SkyLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)