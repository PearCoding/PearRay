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
/// Directional path tracer
/// Based on VCM without connection, merging and direct camera connections
template <bool HasInfLights, VCM::MISMode MISMode>
class IntDiInstance : public IIntegratorInstance {
public:
	using Tracer = VCM::Tracer<false, false, HasInfLights, MISMode>;

	explicit IntDiInstance(const VCM::Options& options, const std::shared_ptr<LightSampler>& lightSampler)
		: mTracer(options, lightSampler)
	{
	}

	virtual ~IntDiInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		VCM::TracerContext tctx(session, mTracer.options());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			mTracer.traceCameraPath(tctx, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));

			// Reset
			tctx.CameraPath.popTokenUntil(1); // Keep first token
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
class IntDi : public IIntegrator {
public:
	explicit IntDi(const VCM::Options& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntDi() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		const bool hasInfLights = !ctx->scene()->infiniteLights().empty();
		if (hasInfLights)
			return std::make_shared<IntDiInstance<true, MISMode>>(mParameters, ctx->lightSampler());
		else
			return std::make_shared<IntDiInstance<false, MISMode>>(mParameters, ctx->lightSampler());
	}

private:
	const VCM::Options mParameters;
};

class IntDiFactory : public IIntegratorFactory {
public:
	explicit IntDiFactory(const ParameterGroup& params)
	{
		size_t maximumDepth = std::numeric_limits<size_t>::max();
		if (params.hasParameter("max_ray_depth"))
			maximumDepth = params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);

		mParameters.MaxCameraRayDepthHard = std::min<size_t>(maximumDepth, params.getUInt("max_camera_ray_depth", mParameters.MaxCameraRayDepthHard));
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_camera_ray_depth", mParameters.MaxCameraRayDepthSoft));

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
			return std::make_shared<IntDi<VCM::MM_Balance>>(mParameters);
		case VCM::MM_Power:
			return std::make_shared<IntDi<VCM::MM_Power>>(mParameters);
		}
	}

private:
	VCM::Options mParameters;
	VCM::MISMode mMISMode;
};

class IntDiPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntDiFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "direct2" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntDiPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)