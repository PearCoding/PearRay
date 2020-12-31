#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "camera/ICamera.h"
#include "emission/IEmission.h"
#include "geometry/Triangle.h"
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
		typename Tracer::IterationContext tctx(iteration, mInitialGatherRadius, session, threadContext, mTracer->options());

		// Construct light paths
		const size_t maxLights = mTracer->options().MaxLightSamples / session.context()->tileCount();
		for (size_t i = 0; i < maxLights; ++i) {
			const SpectralBlob wavelength = VCM::sampleWavelength(session.random());
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
		, mInitialGatherRadius(0)
		, mCBID(0)
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
		// Estimate the average scene camera ray footprint
		static const std::array<Vector2f, 5> primarySamples = {
			Vector2f(0.5f, 0.5f),
			Vector2f(0.25f, 0.25f),
			Vector2f(0.75f, 0.75f),
			Vector2f(0.1f, 0.9f),
			Vector2f(0.9f, 0.1f)
		};

		const auto scene		= ctx->scene();
		const auto camera		= scene->activeCamera();
		const Size2i sensorSize = Size2i(ctx->settings().filmWidth, ctx->settings().filmHeight);

		const auto queryPos = [&](const Vector2f pixel, Vector3f& pos) {
			CameraSample cameraSample;
			cameraSample.SensorSize	   = sensorSize;
			cameraSample.Lens		   = Vector2f(0.5f, 0.5f);
			cameraSample.Time		   = 0;
			cameraSample.BlendWeight   = 1.0f;
			cameraSample.Importance	   = 1.0f;
			cameraSample.WavelengthNM  = SpectralBlob(540.0f);
			cameraSample.WavelengthPDF = 1.0f;
			cameraSample.Pixel		   = pixel;

			const auto camera_ray = camera->constructRay(cameraSample);
			if (!camera_ray.has_value())
				return false;

			Ray ray;
			ray.Origin		   = camera_ray.value().Origin;
			ray.Direction	   = camera_ray.value().Direction;
			ray.MaxT		   = camera_ray.value().MaxT;
			ray.MinT		   = camera_ray.value().MinT;
			ray.WavelengthNM   = camera_ray.value().WavelengthNM;
			ray.IterationDepth = 0;
			ray.GroupID		   = 0;
			ray.PixelIndex	   = 0;
			ray.Flags		   = RF_Camera;

			HitEntry entry;
			if (!scene->traceSingleRay(ray, entry))
				return false;

			pos = ray.t(entry.Parameter[2]);
			return true;
		};

		size_t hits				= 0;
		float acquiredFootprint = 0;
		for (size_t i = 0; i < primarySamples.size(); ++i) {
			constexpr float D	 = 0.25f;
			const Vector2f pixel = Vector2f(primarySamples[i].x() * sensorSize.Width, primarySamples[i].y() * sensorSize.Height);

			Vector3f p00;
			if (!queryPos(pixel + Vector2f(-D, -D), p00))
				continue;

			Vector3f p10;
			if (!queryPos(pixel + Vector2f(D, -D), p10))
				continue;

			Vector3f p01;
			if (!queryPos(pixel + Vector2f(-D, D), p01))
				continue;

			Vector3f p11;
			if (!queryPos(pixel + Vector2f(D, D), p11))
				continue;

			const float area = Triangle::surfaceArea(p00, p10, p01) + Triangle::surfaceArea(p10, p01, p11);
			acquiredFootprint += area / (D * D);
			++hits;
		}

		const auto& bbox = scene->boundingBox();
		if (hits == 0) {
			PR_LOG(L_WARNING) << "Could not acquire average footprint. Using bounding box information instead" << std::endl;
			mInitialGatherRadius = std::max(0.0001f, mParameters.GatherRadiusFactor * bbox.longestEdge() / 100.0f);
		} else {
			// Set initial gather radius based on the footprint
			acquiredFootprint /= hits;
			mInitialGatherRadius = std::max(0.0001f, std::sqrt(acquiredFootprint * PR_INV_PI) * mParameters.GatherRadiusFactor);
		}

		PR_LOG(L_DEBUG) << "VCM: Avg. footprint " << acquiredFootprint << ", initial gather radius " << mInitialGatherRadius << std::endl;
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
			return std::make_shared<IntVCM<VCM::MM_Balance>>(mParameters, mServiceObserver);
		case VCM::MM_Power:
			return std::make_shared<IntVCM<VCM::MM_Power>>(mParameters, mServiceObserver);
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

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntVCMPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)