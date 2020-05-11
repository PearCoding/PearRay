#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Projection.h"
#include "math/Spherical.h"
#include "math/Tangent.h"

#include "sampler/Distribution2D.h"

namespace PR {
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(uint32 id, const std::string& name,
					 const std::shared_ptr<FloatSpectralMapSocket>& spec,
					 const std::shared_ptr<FloatSpectralMapSocket>& background,
					 float factor,
					 const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mRadiance(spec)
		, mBackground(background)
		, mRadianceFactor(factor)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
	{
		Vector2i recSize = spec->queryRecommendedSize();
		if (recSize(0) > 1 && recSize(1) > 1) {
			mDistribution = std::make_unique<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			PR_LOG(L_INFO) << "Generating 2d environment distribution of " << name << std::endl;

			mDistribution->generate([&](size_t x, size_t y) {
				float u = x / (float)recSize(0);
				float v = 1 - y / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * (y + 0.5f) / recSize(1));

				MapSocketCoord coord;
				coord.UV  = Vector2f(u, v);
				coord.dUV = filterSize;

				const float val = sinTheta * mRadiance->relativeLuminance(coord);
				return (val <= PR_EPSILON) ? 0.0f : val;
			});
		}
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		MapSocketCoord coord;
		Vector3f dir = mInvTransform * in.Point.Ray.Direction;

		coord.UV		   = Spherical::uv_from_normal(dir);
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Point.Ray.WavelengthNM;

		if (in.Point.Ray.IterationDepth == 0)
			out.Weight = mBackground->eval(coord);
		else
			out.Weight = mRadiance->eval(coord);

		if (mDistribution) {
			const float sinTheta = std::sin(coord.UV(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			out.PDF_S = Projection::cos_hemi_pdf(std::abs(in.Point.N.dot(dir)));
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		if (mDistribution) {
			Vector2f uv	 = mDistribution->sampleContinuous(in.RND, out.PDF_S);
			out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));
			out.Outgoing = mTransform * Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

			MapSocketCoord coord;
			coord.UV		   = uv;
			coord.UV(1)		   = 1 - coord.UV(1);
			coord.WavelengthNM = in.Point.Ray.WavelengthNM;
			out.Weight		   = mRadianceFactor * mRadiance->eval(coord);

			const float sinTheta = std::sin((1 - uv(1)) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : out.PDF_S / denom;
		} else {
			out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
			out.Outgoing = mTransform * Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

			MapSocketCoord coord;
			coord.UV		   = in.RND;
			coord.UV(1)		   = 1 - coord.UV(1);
			coord.WavelengthNM = in.Point.Ray.WavelengthNM;
			out.Weight		   = mRadianceFactor * mRadiance->eval(coord);
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl
			   << "    Background:   " << (mBackground ? mBackground->dumpInformation() : "NONE") << std::endl
			   << "    Factor:       " << mRadianceFactor << std::endl;
		if (mDistribution) {
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		} else {
			stream << "    Distribution: NONE" << std::endl;
		};

		return stream.str();
	}

private:
	std::unique_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	std::shared_ptr<FloatSpectralMapSocket> mRadiance;
	std::shared_ptr<FloatSpectralMapSocket> mBackground;
	float mRadianceFactor;
	Eigen::Matrix3f mTransform;
	Eigen::Matrix3f mInvTransform;
};

class EnvironmentLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");
		const float factor	   = std::max(0.0000001f, params.getNumber("factor", 1.0f));

		auto rad		 = ctx.Env->lookupSpectralMapSocket(params.getParameter("radiance"), 1);
		auto backgroundP = params.getParameter("background");
		std::shared_ptr<FloatSpectralMapSocket> background;
		if (backgroundP.type() != PT_String)
			background = rad;
		else
			background = ctx.Env->lookupSpectralMapSocket(backgroundP, 1);

		Eigen::Matrix3f trans = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		return std::make_shared<EnvironmentLight>(id, name, rad, background, factor, trans);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "env", "environment", "background" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::EnvironmentLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)