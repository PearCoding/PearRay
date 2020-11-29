#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "renderer/RenderTileSession.h"
#include "sampler/Distribution2D.h"
#include "scene/Scene.h"
#include "shader/NodeUtils.h"
#include "shader/ShadingContext.h"

#include "SampleUtils.h"

namespace PR {
// Four variants available
// -> Distribution + Split
// -> Distribution
// -> Split
// -> None

template <bool UseDistribution, bool UseSplit>
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(const std::shared_ptr<ServiceObserver>& so,
					 const std::string& name, const Transformf& transform,
					 const std::shared_ptr<FloatSpectralNode>& spec,
					 const std::shared_ptr<FloatSpectralNode>& background,
					 const std::shared_ptr<Distribution2D>& distribution)
		: IInfiniteLight(name, transform)
		, mDistribution(distribution)
		, mRadiance(spec)
		, mBackground(background)
		, mSceneRadius(0)
		, mServiceObserver(so)
	{
		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius();
			});
	}

	virtual ~EnvironmentLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		ShadingContext ctx;
		ctx.UV			 = Spherical::uv_from_normal(invNormalMatrix() * in.Direction);
		ctx.WavelengthNM = in.WavelengthNM;

		if constexpr (UseSplit) {
			if (in.IterationDepth == 0)
				out.Radiance = mBackground->eval(ctx);
			else
				out.Radiance = mRadiance->eval(ctx);
		} else {
			out.Radiance = mRadiance->eval(ctx);
		}

		if constexpr (UseDistribution) {
			out.Direction_PDF_S	 = mDistribution->continuousPdf(ctx.UV);
			const float sinTheta = std::sin(ctx.UV(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.Direction_PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			out.Direction_PDF_S = Sampling::cos_hemi_pdf(std::abs((invNormalMatrix() * in.Direction)(2)));
		}
	}

	void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out,
				   const RenderTileSession& session) const override
	{
		Vector2f uv;
		if constexpr (UseDistribution) {
			uv			 = mDistribution->sampleContinuous(in.DirectionRND, out.Direction_PDF_S);
			out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));

			const float sinTheta = std::sin(uv(1) * PR_PI);
			const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
			out.Direction_PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1 / denom;
		} else {
			uv					= in.DirectionRND;
			out.Outgoing		= Sampling::cos_hemi(uv(0), uv(1));
			out.Direction_PDF_S = Sampling::cos_hemi_pdf(out.Outgoing(2));
		}
		out.Outgoing = normalMatrix() * out.Outgoing;

		ShadingContext coord;
		coord.UV		   = uv;
		coord.WavelengthNM = in.WavelengthNM;
		coord.ThreadIndex  = session.threadID();
		out.Radiance	   = mRadiance->eval(coord);
	}

	void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out,
					  const RenderTileSession& session) const override
	{
		sampleDir(in, out, session);

		if (in.Point) { // If we call it outside an intersection point, make light position such that lP - iP = direction
			out.LightPosition  = in.Point->P + mSceneRadius * out.Outgoing;
			out.Position_PDF_A = 1;
		} else {
			out.LightPosition = sampleVisibleHemispherePos(in.PositionRND, out.Outgoing, mSceneRadius, out.Position_PDF_A);
		}
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return NodeUtils::average(wvl, mRadiance.get()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << mRadiance->dumpInformation() << std::endl
			   << "    Background:   " << mBackground->dumpInformation() << std::endl;
		if (mDistribution)
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		else
			stream << "    Distribution: NONE" << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	const std::shared_ptr<FloatSpectralNode> mRadiance;
	const std::shared_ptr<FloatSpectralNode> mBackground;

	float mSceneRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class EnvironmentLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");

		const auto radP				 = params.getParameter("radiance");
		const auto backgroundP		 = params.getParameter("background");
		const bool allowDistribution = params.getBool("distribution", false);
		const bool allowCompensation = params.getBool("compensation", false); // Disabled per default, due to some bugs

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

		const std::shared_ptr<ServiceObserver> so = ctx.environment()->serviceObserver();

		if (dist) {
			if (radiance == background)
				return std::make_shared<EnvironmentLight<true, false>>(so, name, ctx.transform(), radiance, background, dist);
			else
				return std::make_shared<EnvironmentLight<true, true>>(so, name, ctx.transform(), radiance, background, dist);
		} else {
			if (radiance == background)
				return std::make_shared<EnvironmentLight<false, false>>(so, name, ctx.transform(), radiance, background, dist);
			else
				return std::make_shared<EnvironmentLight<false, true>>(so, name, ctx.transform(), radiance, background, dist);
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