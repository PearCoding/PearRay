#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
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

/// Full Vertex Connection and Merging integrator
template <VCM::MISMode MISMode>
class IntVCMInstance : public IIntegratorInstance {
public:
	using Tracer = VCM::Tracer<true, MISMode>;

	explicit IntVCMInstance(Tracer* tracer, const float& initialGatherRadius)
		: mTracer(tracer)
		, mInitialGatherRadius(initialGatherRadius)
	{
	}

	virtual ~IntVCMInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		const size_t iteration = session.context()->currentIteration().Iteration;
		auto& threadContext	   = mTracer->threadContext(session.threadID());
		typename Tracer::IterationContext tctx(iteration, mInitialGatherRadius, session, threadContext, mTracer->options());

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

		const size_t iteration = session.context()->currentIteration().Iteration;
		auto& threadContext	   = mTracer->threadContext(session.threadID());
		Random& rnd			   = session.random(RandomSlot::Light);
		typename Tracer::IterationContext tctx(iteration, mInitialGatherRadius, session, threadContext, mTracer->options());

		// Construct light paths
		const size_t maxLights = mTracer->options().MaxLightSamples / session.context()->tileCount();
		for (size_t i = 0; i < maxLights; ++i) {
			const SpectralBlob wavelength = VCM::sampleWavelength(rnd);
			const Light* light			  = mTracer->traceLightPath(tctx, wavelength);
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
		}
	}

	void onTile(RenderTileSession& session) override
	{
		if (session.context()->currentIteration().Pass == 0)
			lightPass(session);
		else
			cameraPass(session);
	}

private:
	Tracer* mTracer;
	const float& mInitialGatherRadius;
};

template <VCM::MISMode MISMode>
class IntVCM : public IIntegrator {
public:
	using Tracer = VCM::Tracer<true, MISMode>;

	explicit IntVCM(const VCM::Options& parameters, const std::shared_ptr<ServiceObserver>& service)
		: mParameters(parameters)
		, mServiceObserver(service)
		, mCBID(0)
		, mInitialGatherRadius(0)
	{
		if (mServiceObserver) {
			mCBID = mServiceObserver->registerBeforeRender([this](RenderContext* ctx) {
				initialize(ctx);
			});
		}
	}

	virtual ~IntVCM()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		std::lock_guard<std::mutex> lock(mCreationMutex);
		if (!mTracer) {
			mTracer = std::make_unique<Tracer>(mParameters, ctx->lightSampler());

			const BoundingBox bbox = ctx->scene()->boundingBox();

			// Can we be sure about the ctx inside the lambda?
			ctx->addIterationCallback([=](const RenderIteration& iter) {
				if (iter.Pass == 0)
					beforeLightPass();
				else
					beforeCameraPass(bbox);
			});
		}

		mTracer->registerThreadContext();
		return std::make_shared<IntVCMInstance<MISMode>>(mTracer.get(), mInitialGatherRadius);
	}

	IntegratorConfiguration configuration() const override { return IntegratorConfiguration{ 2 /*Pass Count*/ }; }

	inline void initialize(RenderContext* ctx)
	{
		const auto footprint = ctx->computeAverageCameraSceneFootprint();

		if (!footprint.has_value()) {
			PR_LOG(L_WARNING) << "Could not acquire average footprint. Using bounding box information instead" << std::endl;
			mInitialGatherRadius = std::max(0.0001f, mParameters.GatherRadiusFactor * ctx->scene()->boundingBox().longestEdge() / 100.0f);
			PR_LOG(L_DEBUG) << "VCM: Initial gather radius " << mInitialGatherRadius << std::endl;
		} else {
			mInitialGatherRadius = std::max(0.0001f, std::sqrt(footprint.value() * PR_INV_PI) * mParameters.GatherRadiusFactor);
			PR_LOG(L_DEBUG) << "VCM: Avg. footprint " << footprint.value() << ", initial gather radius " << mInitialGatherRadius << std::endl;
		}
	}

private:
	inline void beforeLightPass()
	{
		// Clear previous pass
		for (size_t i = 0; i < mTracer->threadContextCount(); ++i)
			mTracer->threadContext(i).resetLights();
		mTracer->resetLights();
	}

	inline void beforeCameraPass(const BoundingBox& bbox)
	{
		constexpr size_t MAX_ELEMS = 256;

		// Make sure the memory use is reasonable
		const float gridDelta = std::max(bbox.longestEdge() / float(MAX_ELEMS), 2 * mInitialGatherRadius);
		mTracer->setupSearchGrid(bbox, gridDelta);
		mTracer->setupWavelengthSelector();
	}

	const VCM::Options mParameters;
	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;

	std::mutex mCreationMutex;
	std::unique_ptr<Tracer> mTracer;
	float mInitialGatherRadius;
};

class IntVCMFactory : public IIntegratorFactory {
public:
	explicit IntVCMFactory(const ParameterGroup& params, const std::shared_ptr<ServiceObserver>& service)
		: mServiceObserver(service)
	{
		size_t maximumDepth = std::numeric_limits<size_t>::max();
		if (params.hasParameter("max_ray_depth"))
			maximumDepth = params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);

		mParameters.MaxCameraRayDepthHard = std::min<size_t>(maximumDepth, params.getUInt("max_camera_ray_depth", mParameters.MaxCameraRayDepthHard));
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_camera_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = std::min<size_t>(maximumDepth, params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard));
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));

		mParameters.MaxLightSamples	   = std::max<size_t>(100, params.getUInt("max_light_samples", mParameters.MaxLightSamples));
		mParameters.GatherRadiusFactor = std::max(0.00001f, params.getNumber("gather_radius_factor", mParameters.GatherRadiusFactor));

		mParameters.SqueezeWeight2 = std::max(0.0f, std::min(1.0f, params.getNumber("squeeze_weight", std::sqrt(mParameters.SqueezeWeight2))));
		mParameters.SqueezeWeight2 *= mParameters.SqueezeWeight2;

		mParameters.ContractRatio = std::max(0.0f, std::min(1.0f, params.getNumber("contract_ratio", mParameters.ContractRatio)));

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
			return std::make_shared<IntVCM<VCM::MISMode::Balance>>(mParameters, mServiceObserver);
		case VCM::MISMode::Power:
			return std::make_shared<IntVCM<VCM::MISMode::Power>>(mParameters, mServiceObserver);
		}
	}

private:
	VCM::Options mParameters;
	VCM::MISMode mMISMode;
	const std::shared_ptr<ServiceObserver> mServiceObserver;
};

class IntVCMPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntVCMFactory>(ctx.parameters(), ctx.environment()->serviceObserver());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "vcm", "merging" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		const VCM::Options parameters;
		return PluginSpecificationBuilder("VCM", "Vertex Connection and Merging")
			.Identifiers(getNames())
			.Inputs()
			.UInt("max_ray_depth", "Maximum ray depth allowed for both camera and light rays", parameters.MaxCameraRayDepthHard)
			.UInt("max_camera_ray_depth", "Maximum ray depth allowed for camera rays", parameters.MaxCameraRayDepthHard)
			.UInt("soft_max_camera_ray_depth", "Maximum ray depth after which russian roulette for camera rays starts", parameters.MaxCameraRayDepthSoft)
			.UInt("max_light_ray_depth", "Maximum ray depth allowed for light rays", parameters.MaxLightRayDepthHard)
			.UInt("soft_max_light_ray_depth", "Maximum ray depth after which russian roulette for light rays starts", parameters.MaxLightRayDepthSoft)
			.UInt("max_light_samples", "Maximum number of light samples to trace", parameters.MaxLightSamples)
			.Number("gather_radius_factor", "Factor for automatic initial gather radius calculation", parameters.GatherRadiusFactor)
			.Number("squeeze_weight", "Squeeze weight to prevent surface leaks", std::sqrt(parameters.SqueezeWeight2))
			.Number("contract_ratio", "Contract ratio", parameters.ContractRatio)
			.Option("mis", "MIS mode", "balance", { "balance", "power" })
			.Specification()
			.get();
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntVCMPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)