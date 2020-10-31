#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "sampler/Distribution2D.h"
#include "scene/Scene.h"
#include "shader/NodeUtils.h"
#include "shader/ShadingContext.h"

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
		, mSceneRadius(0)
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		ShadingContext ctx;
		ctx.UV			 = Spherical::uv_from_normal(mInvTransform * in.Ray.Direction);
		ctx.WavelengthNM = in.Ray.WavelengthNM;

		if constexpr (UseSplit) {
			if (in.Ray.IterationDepth == 0)
				out.Radiance = mBackground->eval(ctx);
			else
				out.Radiance = mRadiance->eval(ctx);
		} else {
			out.Radiance = mRadiance->eval(ctx);
		}

		if constexpr (UseDistribution) {
			out.PDF_S			 = mDistribution->continuousPdf(ctx.UV);
			const float sinTheta = std::sin(ctx.UV(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			out.PDF_S = in.Point ? Sampling::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(in.Ray.Direction))) : 1.0f;
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		Vector2f uv;
		if constexpr (UseDistribution) {
			uv			 = mDistribution->sampleContinuous(Vector2f(in.RND(0), in.RND(1)), out.PDF_S);
			out.Outgoing = mTransform * Spherical::cartesian_from_uv(uv(0), uv(1));

			const float sinTheta = std::sin(uv(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1 / denom;
		} else {
			uv			 = Vector2f(in.RND(0), in.RND(1));
			out.Outgoing = Sampling::cos_hemi(uv(0), uv(1));
			out.PDF_S	 = Sampling::cos_hemi_pdf(out.Outgoing(2));
		}

		ShadingContext coord;
		coord.UV		   = uv;
		coord.WavelengthNM = in.WavelengthNM;
		out.Radiance	   = mRadiance->eval(coord);

		if (in.Point) // If we call it outside an intersection point, make light position such that lP - iP = direction
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		else if (in.SamplePosition) {
			out.LightPosition = 2 * mSceneRadius * out.Outgoing;
			// Instead of sampling position, sample direction again
			constexpr float CosAtan05 = 0.894427190999915f; // cos(atan(0.5)) = 2/sqrt(5)
			const Vector3f local	  = Sampling::uniform_cone(in.RND(2), in.RND(3), CosAtan05);
			out.Outgoing			  = Tangent::align(out.Outgoing, local);
			out.PDF_S *= Sampling::uniform_cone_pdf(CosAtan05) / mSceneRadius;
		}
	}

	float power() const override { return NodeUtils::average(SpectralBlob(550.0f) /*TODO*/, mRadiance.get()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl
			   << "    Background:   " << (mBackground ? mBackground->dumpInformation() : "NONE") << std::endl;
		if (mDistribution)
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		else
			stream << "    Distribution: NONE" << std::endl;

		return stream.str();
	}

	void afterSceneBuild(Scene* scene) override
	{
		IInfiniteLight::afterSceneBuild(scene);
		mSceneRadius = scene->boundingSphere().radius() * 1.05f /*Scale a little bit*/;
	}

private:
	const std::shared_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	const std::shared_ptr<FloatSpectralNode> mRadiance;
	const std::shared_ptr<FloatSpectralNode> mBackground;
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;

	float mSceneRadius;
};

class EnvironmentLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");

		const auto radP				 = params.getParameter("radiance");
		const auto backgroundP		 = params.getParameter("background");
		const bool allowDistribution = params.getBool("distribution", true);
		const bool allowCompensation = params.getBool("compensation", true);

		std::shared_ptr<FloatSpectralNode> radiance;
		std::shared_ptr<FloatSpectralNode> background;
		if (radP.isValid() && backgroundP.isValid()) {
			radiance   = ctx.lookupSpectralNode(radP, 1);
			background = ctx.lookupSpectralNode(backgroundP, 1);
		} else if (radP.isValid()) {
			radiance   = ctx.lookupSpectralNode(radP, 1);
			background = radiance;
		} else {
			background = ctx.lookupSpectralNode(backgroundP, 1);
			radiance   = background;
		}

		Eigen::Matrix3f trans = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		Vector2i recSize = radiance->queryRecommendedSize();
		std::shared_ptr<Distribution2D> dist;
		if (allowDistribution && recSize(0) > 1 && recSize(1) > 1) {
			dist = std::make_shared<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			PR_LOG(L_INFO) << "Generating 2d environment (" << dist->width() << "x" << dist->height() << ") distribution of " << name << std::endl;

			dist->generate([&](size_t x, size_t y) {
				float u = (x + 0.5f) / (float)recSize(0);
				float v = (y + 0.5f) / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * v);

				ShadingContext coord;
				coord.UV		   = Vector2f(u, v);
				coord.dUV		   = filterSize;
				coord.WavelengthNM = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test

				const float val = sinTheta * radiance->eval(coord).maxCoeff();
				return (val <= PR_EPSILON) ? 0.0f : val;
			});

			if (allowCompensation)
				dist->applyCompensation();
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