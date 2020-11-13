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
template <bool HasInfLights, VCM::MISMode MISMode>
class IntBiDiInstance : public IIntegratorInstance {
public:
	using Tracer = VCM::Tracer<true, false, HasInfLights, MISMode>;

	explicit IntBiDiInstance(const VCM::Options& options, const std::shared_ptr<LightSampler>& lightSampler)
		: mTracer(options, lightSampler)
	{
	}

	virtual ~IntBiDiInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		VCM::TracerContext tctx(session, mTracer.options());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			// Trace necessary paths
			const Light* light = mTracer.traceLightPath(tctx, spt.Ray.WavelengthNM);
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
			PR_ASSERT(tctx.LightPath.currentSize() > tctx.LightVertices.size(), "Light vertices and path do not match");

			mTracer.traceCameraPath(tctx, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));

			// Reset
			tctx.CameraPath.popTokenUntil(1); // Keep first token
			tctx.LightPath.popTokenUntil(0);  // Do NOT keep first token!
			tctx.LightVertices.clear();
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
	const Tracer mTracer;
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
		const bool hasInfLights = !ctx->scene()->infiniteLights().empty();
		if (hasInfLights)
			return std::make_shared<IntBiDiInstance<true, MISMode>>(mParameters, ctx->lightSampler());
		else
			return std::make_shared<IntBiDiInstance<false, MISMode>>(mParameters, ctx->lightSampler());
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
			mMISMode = VCM::MM_Power;
		else
			mMISMode = VCM::MM_Balance;
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mMISMode) {
		default:
		case VCM::MM_Balance:
			return std::make_shared<IntBiDi<VCM::MM_Balance>>(mParameters);
		case VCM::MM_Power:
			return std::make_shared<IntBiDi<VCM::MM_Power>>(mParameters);
		}
	}

private:
	VCM::Options mParameters;
	VCM::MISMode mMISMode;
};

class IntBiDiPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
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