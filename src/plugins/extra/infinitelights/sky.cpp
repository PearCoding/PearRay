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
	SkyLight(uint32 id, const std::string& name, const SkyModel& model)
		: IInfiniteLight(id, name)
		, mDistribution()
		, mModel(model)
	{
		buildDistribution();
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		ShadingContext ctx;
		Vector3f dir = in.Ray.Direction;

		ctx.UV			 = Spherical::uv_from_normal(dir);
		ctx.UV(1)		 = 1 - ctx.UV(1);
		ctx.WavelengthNM = in.Ray.WavelengthNM;

		out.Weight = radiance(ctx.WavelengthNM, ctx.UV);

		const float sinTheta = std::sin(ctx.UV(1) * PR_PI);
		const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
		out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		Vector2f uv	 = mDistribution->sampleContinuous(in.RND, out.PDF_S);
		out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));

		const float sinTheta = std::sin((1 - uv(1)) * PR_PI);
		const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
		out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : out.PDF_S / denom;

		out.Outgoing = Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);

		ShadingContext coord;
		coord.UV		   = uv;
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Point.Ray.WavelengthNM;
		out.Weight		   = radiance(coord.WavelengthNM, coord.UV);
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
		mDistribution = std::make_shared<Distribution2D>(mModel.phiCount(), mModel.thetaCount());

		const Vector2f filterSize(1.0f / mModel.phiCount(), 1.0f / mModel.thetaCount());

		PR_LOG(L_INFO) << "Generating 2d environment ("<< mDistribution->width() << "x" << mDistribution->height() << ") of " << name() << std::endl;

		mDistribution->generate([&](size_t x, size_t y) {
			float u = x / (float)mModel.phiCount();
			float v = 1 - y / (float)mModel.thetaCount();

			float sinTheta = std::sin(PR_PI * (y + 0.5f) / mModel.thetaCount());

			ShadingContext coord;
			coord.UV		   = Vector2f(u, v);
			coord.dUV		   = filterSize;
			coord.WavelengthNM = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test

			const float val = sinTheta * radiance(coord.WavelengthNM, coord.UV).maxCoeff();
			return (val <= PR_EPSILON) ? 0.0f : val;
		});
	}

	inline SpectralBlob radiance(const SpectralBlob& wvls, const Vector2f& uv) const
	{
		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float f	= std::min<float>(AR_SPECTRAL_BANDS - 2, std::max(0.0f, (wvls[i] - AR_SPECTRAL_START) / AR_SPECTRAL_DELTA));
			const int index = f;
			const float t	= f - index;

			float radiance = mModel.radiance(index, uv(1), uv(0)) * (1 - t) + mModel.radiance(index + 1, uv(1), uv(0)) * t;
			blob[i]		   = radiance;
		}
		return blob;
	}

	std::shared_ptr<Distribution2D> mDistribution;
	const SkyModel mModel;
};

class SkyLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");

		auto ground_albedo	   = ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("albedo"), 0.2f);
		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		PR_LOG(L_INFO) << "Sun: " << sunEA.Elevation << " " << sunEA.Azimuth << std::endl;

		return std::make_shared<SkyLight>(id, name, SkyModel(ground_albedo, sunEA, ctx.Parameters));
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