#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "light/LightSampler.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "math/Sampling.h"
#include "output/Feedback.h"
#include "output/OutputSystem.h"
#include "path/LightPath.h"
#include "photon/PhotonMap.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "sampler/SampleArray.h"
#include "trace/IntersectionPoint.h"

#include "IntegratorUtils.h"
#include "vcm/Tracer.h"

#include "Logger.h"

namespace PR {
/// Bidirectional path tracer
/// Based on VCM without merging and direct camera connections
template <VCM::MISMode MISMode>
class IntBiDiInstance : public IIntegratorInstance {
public:
	using Tracer = VCM::Tracer<false, MISMode>;

	explicit IntBiDiInstance(const VCM::Options& options, const std::shared_ptr<LightSampler>& lightSampler)
		: mTracer(options, lightSampler)
	{
	}

	virtual ~IntBiDiInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		typename Tracer::ThreadContext threadContext(mTracer.options());
		typename Tracer::IterationContext tctx(session, threadContext, mTracer.options());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			// Trace necessary paths
			const Light* light = mTracer.traceLightPath(tctx, spt.Ray.WavelengthNM);
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
			PR_ASSERT(threadContext.LightPath.currentSize() >= threadContext.BDPTLightVertices.size(), "Light vertices and path do not match");

			mTracer.traceCameraPath(tctx, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));

			// Reset
			threadContext.resetCamera();
			threadContext.resetLights();
		}
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;
		while (!session.pipeline()->isFinished()) {
			session.pipeline()->runPipeline();
			while (session.pipeline()->hasShadingGroup()) {
				auto sg = session.pipeline()->popShadingGroup(session);
				if (sg.isBackground())
					IntegratorUtils::handleBackgroundGroup(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	Tracer mTracer;
};

template <VCM::MISMode MISMode>
class IntBiDi : public IIntegrator {
public:
	explicit IntBiDi(const VCM::Options& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntBiDi() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntBiDiInstance<MISMode>>(mParameters, ctx->lightSampler());
	}

private:
	const VCM::Options mParameters;
};

class IntBiDiFactory : public IIntegratorFactory {
public:
	explicit IntBiDiFactory(const ParameterGroup& params)
	{
		size_t maximumDepth = std::numeric_limits<size_t>::max();
		if (params.hasParameter("max_ray_depth"))
			maximumDepth = params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);

		mParameters.MaxCameraRayDepthHard = std::min<size_t>(maximumDepth, params.getUInt("max_camera_ray_depth", mParameters.MaxCameraRayDepthHard));
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_camera_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = std::min<size_t>(maximumDepth, params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard));
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));

		std::string mode = params.getString("mis", "balance");
		std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
		if (mode == "power")
			mMISMode = VCM::MISMode::Power;
		else
			mMISMode = VCM::MISMode::Balance;
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mMISMode) {
		default:
		case VCM::MISMode::Balance:
			return std::make_shared<IntBiDi<VCM::MISMode::Balance>>(mParameters);
		case VCM::MISMode::Power:
			return std::make_shared<IntBiDi<VCM::MISMode::Power>>(mParameters);
		}
	}

private:
	VCM::Options mParameters;
	VCM::MISMode mMISMode;
};

class IntBiDiPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntBiDiFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "bidi", "bdpt", "bidirectional", "bidirect" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntBiDiPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)