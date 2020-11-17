#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "buffer/Feedback.h"
#include "buffer/FrameBufferSystem.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "light/LightSampler.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "math/Sampling.h"
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
class IntVCMInstance : public IIntegratorInstance {
public:
	using Tracer = VCM::Tracer<true, MISMode>;

	explicit IntVCMInstance(Tracer* tracer)
		: mTracer(tracer)
	{
	}

	virtual ~IntVCMInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		const size_t iteration = session.tile()->iterationCount() / 2;
		auto& threadContext	   = mTracer->threadContext(session.threadID());
		typename Tracer::IterationContext tctx(iteration, session, threadContext, mTracer->options());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			mTracer->traceCameraPath(tctx, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));

			// Reset
			threadContext.resetCamera();
		}
	}

	void cameraPass(RenderTileSession& session)
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

	void lightPass(RenderTileSession& session)
	{
		PR_PROFILE_THIS;

		const size_t iteration = session.tile()->iterationCount() / 2;
		auto& threadContext	   = mTracer->threadContext(session.threadID());
		typename Tracer::IterationContext tctx(iteration, session, threadContext, mTracer->options());

		// Construct light paths
		const size_t maxLights = (mTracer->options().MaxLightSamples * mTracer->threadContextCount()) / session.context()->tileCount();
		for (size_t i = 0; i < maxLights; ++i) {
			const SpectralBlob wavelength = VCM::sampleWavelength(session.random());
			const Light* light			  = mTracer->traceLightPath(tctx, wavelength);
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
		}
	}

	void onTile(RenderTileSession& session) override
	{
		const bool isCameraPass = session.tile()->iterationCount() % 2 != 0;

		if (!isCameraPass)
			lightPass(session);
		else
			cameraPass(session);
	}

private:
	Tracer* mTracer;
};

template <VCM::MISMode MISMode>
class IntVCM : public IIntegrator {
public:
	using Tracer = VCM::Tracer<true, MISMode>;

	explicit IntVCM(const VCM::Options& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntVCM() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		std::lock_guard<std::mutex> lock(mCreationMutex);
		if (!mTracer) {
			VCM::Options options = mParameters;
			options.MaxLightSamples /= ctx->threadCount(); // Make sure all threads together sum up to the original count
			mTracer = std::make_unique<Tracer>(mParameters, ctx->lightSampler());

			ctx->addIterationCallback([this](uint32 iter) {
				if (iter % 2 == 0)
					beforeLightPass(iter / 2);
				else
					beforeCameraPass(iter / 2);
			});
		}

		const BoundingBox bbox		 = ctx->scene()->boundingBox();
		const float scene_grid_delta = std::max(0.0001f, mParameters.MaxGatherRadius * bbox.longestEdge());

		mTracer->registerThreadContext(bbox, scene_grid_delta);
		return std::make_shared<IntVCMInstance<MISMode>>(mTracer.get());
	}

private:
	inline void beforeLightPass(size_t)
	{
		// Clear previous pass
		for (size_t i = 0; i < mTracer->threadContextCount(); ++i)
			mTracer->threadContext(i).resetLights();
	}

	inline void beforeCameraPass(size_t)
	{
		// Construct search/neighrest neighbor structure
		for (size_t i = 0; i < mTracer->threadContextCount(); ++i)
			mTracer->setupSearchGrid(mTracer->threadContext(i));
	}

	const VCM::Options mParameters;
	std::mutex mCreationMutex;
	std::unique_ptr<Tracer> mTracer;
};

class IntVCMFactory : public IIntegratorFactory {
public:
	explicit IntVCMFactory(const ParameterGroup& params)
	{
		size_t maximumDepth = std::numeric_limits<size_t>::max();
		if (params.hasParameter("max_ray_depth"))
			maximumDepth = params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);

		mParameters.MaxCameraRayDepthHard = std::min<size_t>(maximumDepth, params.getUInt("max_camera_ray_depth", mParameters.MaxCameraRayDepthHard));
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_camera_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = std::min<size_t>(maximumDepth, params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard));
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));

		mParameters.MaxLightSamples = std::max<size_t>(100, params.getUInt("max_light_samples", mParameters.MaxLightSamples));
		mParameters.MaxGatherRadius = std::max(0.00001f, params.getNumber("max_gather_radius", mParameters.MaxGatherRadius));

		mParameters.SqueezeWeight2 = std::max(0.0f, std::min(1.0f, params.getNumber("squeeze_weight", mParameters.SqueezeWeight2)));
		mParameters.SqueezeWeight2 *= mParameters.SqueezeWeight2;

		mParameters.ContractRatio = std::max(0.0f, std::min(1.0f, params.getNumber("contract_ratio", mParameters.ContractRatio)));

		std::string mode = params.getString("mis", "balance");
		std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
		if (mode == "power")
			mMISMode = VCM::MM_Power;
		else
			mMISMode = VCM::MM_Balance;
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mMISMode) {
		default:
		case VCM::MM_Balance:
			return std::make_shared<IntVCM<VCM::MM_Balance>>(mParameters);
		case VCM::MM_Power:
			return std::make_shared<IntVCM<VCM::MM_Power>>(mParameters);
		}
	}

private:
	VCM::Options mParameters;
	VCM::MISMode mMISMode;
};

class IntVCMPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntVCMFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "vcm", "merging" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntVCMPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)