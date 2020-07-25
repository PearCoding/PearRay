#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "shader/ShadingContext.h"

#include "sampler/Distribution2D.h"

namespace PR {
// Four variants available
// -> Distribution + Split
// -> Distribution
// -> Split
// -> None

template <bool UseDistribution, bool UseSplit>
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(uint32 id, const std::string& name,
					 const std::shared_ptr<FloatSpectralNode>& spec,
					 const std::shared_ptr<FloatSpectralNode>& background,
					 const std::shared_ptr<Distribution2D>& distribution,
					 const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mDistribution(distribution)
		, mRadiance(spec)
		, mBackground(background)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		ShadingContext ctx;
		Vector3f dir = mInvTransform * in.Ray.Direction;

		ctx.UV			 = Spherical::uv_from_normal(dir);
		ctx.UV(1)		 = 1 - ctx.UV(1);
		ctx.WavelengthNM = in.Ray.WavelengthNM;

		if constexpr (UseSplit) {
			if (in.Ray.IterationDepth == 0)
				out.Weight = mBackground->eval(ctx);
			else
				out.Weight = mRadiance->eval(ctx);
		} else {
			out.Weight = mRadiance->eval(ctx);
		}

		if constexpr (UseDistribution) {
			const float sinTheta = std::sin(ctx.UV(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			out.PDF_S = in.Point ? Sampling::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(dir))) : 1.0f;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		Vector2f uv;
		if constexpr (UseDistribution) {
			uv			 = mDistribution->sampleContinuous(in.RND, out.PDF_S);
			out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));

			const float sinTheta = std::sin((1 - uv(1)) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : out.PDF_S / denom;
		} else {
			uv			 = in.RND;
			out.Outgoing = Sampling::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
		}
		out.Outgoing = mTransform * Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);

		ShadingContext coord;
		coord.UV		   = uv;
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Point.Ray.WavelengthNM;
		out.Weight		   = mRadiance->eval(coord);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl
			   << "    Background:   " << (mBackground ? mBackground->dumpInformation() : "NONE") << std::endl;
		if (mDistribution) {
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		} else {
			stream << "    Distribution: NONE" << std::endl;
		};

		return stream.str();
	}

private:
	const std::shared_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	const std::shared_ptr<FloatSpectralNode> mRadiance;
	const std::shared_ptr<FloatSpectralNode> mBackground;
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;
};

class EnvironmentLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");

		auto radP			 = params.getParameter("radiance");
		auto backgroundP	 = params.getParameter("background");
		const bool allowDist = params.getBool("distribution", true);

		std::shared_ptr<FloatSpectralNode> radiance;
		std::shared_ptr<FloatSpectralNode> background;
		if (radP.isValid() && backgroundP.isValid()) {
			radiance   = ctx.Env->lookupSpectralNode(radP, 1);
			background = ctx.Env->lookupSpectralNode(backgroundP, 1);
		} else if (radP.isValid()) {
			radiance   = ctx.Env->lookupSpectralNode(radP, 1);
			background = radiance;
		} else {
			background = ctx.Env->lookupSpectralNode(backgroundP, 1);
			radiance   = background;
		}

		Eigen::Matrix3f trans = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		Vector2i recSize = radiance->queryRecommendedSize();
		std::shared_ptr<Distribution2D> dist;
		if (allowDist && recSize(0) > 1 && recSize(1) > 1) {
			dist = std::make_shared<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			PR_LOG(L_INFO) << "Generating 2d environment (" << dist->width() << "x" << dist->height() << ") distribution of " << name << std::endl;

			dist->generate([&](size_t x, size_t y) {
				float u = x / (float)recSize(0);
				float v = 1 - y / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * (y + 0.5f) / recSize(1));

				ShadingContext coord;
				coord.UV		   = Vector2f(u, v);
				coord.dUV		   = filterSize;
				coord.WavelengthNM = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test

				const float val = sinTheta * radiance->eval(coord).maxCoeff();
				return (val <= PR_EPSILON) ? 0.0f : val;
			});
		}

		if (dist) {
			if (radiance == background)
				return std::make_shared<EnvironmentLight<true, false>>(id, name, radiance, background, dist, trans);
			else
				return std::make_shared<EnvironmentLight<true, true>>(id, name, radiance, background, dist, trans);
		} else {
			if (radiance == background)
				return std::make_shared<EnvironmentLight<false, false>>(id, name, radiance, background, dist, trans);
			else
				return std::make_shared<EnvironmentLight<false, true>>(id, name, radiance, background, dist, trans);
		}
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