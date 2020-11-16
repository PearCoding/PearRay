#include "Environment.h"
#include "IntegratorUtils.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "buffer/Feedback.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "light/LightSampler.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "path/LightPath.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "sampler/SampleArray.h"
#include "trace/IntersectionPoint.h"
#include "vcm/MIS.h"
#include "vcm/RussianRoulette.h"
#include "vcm/Utils.h"
#include "vcm/Walker.h"

namespace PR {

struct DiParameters {
	size_t MaxCameraRayDepthHard = 64;
	size_t MaxCameraRayDepthSoft = 4;
};

/// Standard path tracing
/// Suports NEE, Inf Lights
/// TODO: Mediums/Volume
/// TODO: Revisit Hero Wavelength MIS
template <bool HasInfLights, VCM::MISMode MISMode>
class IntDirectInstance : public IIntegratorInstance {
private:
	struct TraversalContext {
		SpectralBlob Throughput;
		bool LastWasDelta;
		float LastPDF_S;
		Vector3f LastPosition; // Used in NEE
		Vector3f LastNormal;
	};

public:
	explicit IntDirectInstance(const DiParameters& parameters, const std::shared_ptr<LightSampler>& lightSampler)
		: mParameters(parameters)
		, mLightSampler(lightSampler)
		, mCameraRR(parameters.MaxCameraRayDepthSoft)
		, mCameraWalker(mParameters.MaxCameraRayDepthHard)
		, mCameraPath(mParameters.MaxCameraRayDepthHard + 2)
	{
		mCameraPath.addToken(LightPathToken::Camera());
	}

	virtual ~IntDirectInstance() = default;

	// Every camera vertex
	std::optional<Ray> handleCameraVertex(RenderTileSession& session, const IntersectionPoint& ip,
										  IEntity* entity, IMaterial* material,
										  TraversalContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		PR_PROFILE_THIS;

		const uint32 pathLength = ip.Ray.IterationDepth + 1;

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addCameraDepthCount();

		if (pathLength == 1)
			session.pushSPFragment(ip, mCameraPath);

		if (entity->hasEmission()) {
			mCameraPath.addToken(LightPathToken::Emissive());
			handleDirectHit(session, ip, entity, current);
			mCameraPath.popToken();
		}

		// If there is no material to scatter from, give up
		if (PR_UNLIKELY(!material))
			return {};

		if (!material->hasDeltaDistribution())
			handleNEE(session, ip, material, current);

		return handleScattering(session, ip, entity, material, current);
	}

	// First camera vertex
	void traceCameraPath(RenderTileSession& session, const IntersectionPoint& initial_hit,
						 IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Initial camera vertex
		TraversalContext current = TraversalContext{ SpectralBlob::Ones(), true, 1, Vector3f::Zero(), Vector3f::Zero() };

		mCameraWalker.traverse(
			session, initial_hit, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(session, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				if constexpr (HasInfLights) {
					mCameraPath.addToken(LightPathToken::Background());
					handleInfLights(session, current, ray);
					mCameraPath.popToken();
				} else {
					PR_UNUSED(ray);
				}
			});

		mCameraPath.popTokenUntil(1);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);
			traceCameraPath(session, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
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
	std::optional<Ray> handleScattering(RenderTileSession& session, const IntersectionPoint& ip,
										IEntity* entity, IMaterial* material, TraversalContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd = session.random();

		current.LastPosition = ip.P;
		current.LastNormal	 = ip.Surface.N;

		// Russian roulette
		const auto roulette = mCameraRR.check(session.random(), ip.Ray.IterationDepth + 1);
		if (!roulette.has_value())
			return {};

		const float scatProb = roulette.value();

		// TODO: Add volume support
		if (!material)
			return {};

		// Sample Material
		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		mCameraPath.addToken(sout.Type);

		const Vector3f L	 = sout.globalL(ip);
		const bool isDelta	 = material->hasDeltaDistribution();
		current.LastWasDelta = isDelta;

		const float pdf_s = sout.PDF_S[0] * scatProb;
		current.LastPDF_S = pdf_s;

		if (pdf_s <= PR_EPSILON) // Catch this case, or everything will explode
			return {};

		// Update throughput
		current.Throughput *= sout.Weight / pdf_s; // cosinus term already applied inside material

		if (material->isSpectralVarying())
			current.Throughput *= SpectralBlobUtils::HeroOnly();

		if (current.Throughput.isZero(PR_EPSILON))
			return {};

		// Setup ray flags
		int rflags = RF_Bounce;
		if (material->isSpectralVarying())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.nextRay(L, rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	void handleNEE(RenderTileSession& session, const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, TraversalContext& current)
	{
		const EntitySamplingInfo sampleInfo = { cameraIP.P, cameraIP.Surface.N };

		// Sample light
		LightSampleInput lsin;
		lsin.RND			= session.random().get4D();
		lsin.WavelengthNM	= cameraIP.Ray.WavelengthNM;
		lsin.Point			= &cameraIP;
		lsin.SamplingInfo	= &sampleInfo;
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, session);
		const Light* light = lsample.first;
		if (PR_UNLIKELY(!light))
			return;

		// Calculate geometry stuff
		const float sqrD	  = (lsout.LightPosition - cameraIP.P).squaredNorm();
		const Vector3f L	  = lsout.Outgoing;
		const float cosC	  = std::abs(L.dot(cameraIP.Surface.N));
		const float cosL	  = culling(lsout.CosLight);
		const float Geometry  = cosC * cosL / sqrD;
		const bool isFeasible = Geometry > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		if (!isFeasible) // MIS is zero
			return;

		// Evaluate camera material
		MaterialEvalInput min;
		min.Context		   = MaterialEvalContext::fromIP(cameraIP, L);
		min.ShadingContext = ShadingContext::fromIP(session.threadID(), cameraIP);
		MaterialEvalOutput mout;
		cameraMaterial->eval(min, mout, session);

		// Evaluate direct pdf
		float lightPdfS = 0;
		if (light->isInfinite()) {
			lightPdfS = lsout.Direction_PDF_S;
		} else {
			// Convert position pdf to solid angle if necessary
			lightPdfS = lsout.Position_PDF.Value;
			if (lsout.Position_PDF.IsArea)
				lightPdfS = IS::toSolidAngle(lightPdfS, sqrD, cosL);
		}
		lightPdfS *= lsample.second;

		// Calculate MIS
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const float cameraRoulette	  = mCameraRR.probability(cameraPathLength);

		const float bsdfPdfS = mout.PDF_S[0] * cameraRoulette;
		const float mis		 = light->hasDeltaDistribution() ? 1 : 1 / (1 + VCM::mis_term<MISMode>(bsdfPdfS / lightPdfS));

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Ray shadow	 = cameraIP.nextRay(L, RF_Shadow, SHADOW_RAY_MIN, distance);

		const bool isVisible	  = !session.traceShadowRay(shadow, distance);
		const SpectralBlob lightW = lsout.Radiance;

		// Calculate contribution (cosinus term already applied inside material)
		const SpectralBlob contrib = isVisible ? (lightW * mout.Weight / lightPdfS).eval() : SpectralBlob::Zero();

		// Construct LPE path
		mCameraPath.addToken(mout.Type);

		if (light->isInfinite())
			mCameraPath.addToken(LightPathToken::Background());
		else
			mCameraPath.addToken(LightPathToken::Emissive());

		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib,
									 cameraIP.Ray, mCameraPath);

		mCameraPath.popToken(2);
	}

	// Handle case where camera ray directly hits emissive object
	void handleDirectHit(RenderTileSession& session, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, TraversalContext& current)
	{
		const EntitySamplingInfo sampleInfo = { current.LastPosition, current.LastNormal };
		const uint32 cameraPathLength		= cameraIP.Ray.IterationDepth + 1;

		const IEmission* ems = session.getEmission(cameraIP.Surface.Geometry.EmissionID);
		if (PR_UNLIKELY(!ems))
			return;

		// Cull emission if needed
		const float cosC = -cameraIP.Surface.NdotV;
		if (std::abs(cosC) <= PR_EPSILON)
			return;

		SpectralBlob radiance;
		if (cosC < 0.0f) {
			radiance = SpectralBlob::Zero();
		} else {
			// Evaluate emission
			EmissionEvalInput ein;
			ein.Entity		   = cameraEntity;
			ein.ShadingContext = ShadingContext::fromIP(session.threadID(), cameraIP);
			EmissionEvalOutput eout;
			ems->eval(ein, eout, session);
			radiance = eout.Radiance;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1 || current.LastWasDelta) {
			session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, cameraIP.Ray, mCameraPath);
			return;
		}

		// TODO: Something wrong here!
		//return;

		// Evaluate PDF
		const float selProb = mLightSampler->pdfEntitySelection(cameraEntity);
		auto posPDF			= mLightSampler->pdfPosition(cameraEntity, cameraIP.P, &sampleInfo);
		if (posPDF.IsArea)
			posPDF.Value = IS::toSolidAngle(posPDF.Value, cameraIP.Depth2, std::abs(cosC));
		const float posPDF_S = posPDF.Value * selProb;

		// Calculate MIS
		const float mis = 1 / (1 + VCM::mis_term<MISMode>(posPDF_S / current.LastPDF_S));

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, radiance, cameraIP.Ray, mCameraPath);
	}

	// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const RenderTileSession& session, TraversalContext& current, const Ray& ray) const
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;

		// Evaluate radiance
		float denom_mis		  = 0;
		SpectralBlob radiance = SpectralBlob::Zero();
		for (auto light : mLightSampler->infiniteLights()) {
			if (light->hasDeltaDistribution())
				continue;

			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->asInfiniteLight()->eval(lin, lout, session);

			const float selProb = mLightSampler->pdfLightSelection(light);
			const float pdf_S	= lout.Direction_PDF_S * selProb;
			radiance += lout.Radiance;
			denom_mis += VCM::mis_term<MISMode>(pdf_S / current.LastPDF_S);
		}

		// If directly visible from camera or last one was a delta distribution, do not calculate mis weights
		if (cameraPathLength == 1 || current.LastWasDelta) {
			session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, ray, mCameraPath);
			return;
		}

		// Calculate MIS
		const float mis = 1 / (1 + denom_mis);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, radiance, ray, mCameraPath);
	}

private:
	const DiParameters mParameters;
	const std::shared_ptr<LightSampler> mLightSampler;
	const RussianRoulette mCameraRR;
	const Walker mCameraWalker;

	LightPath mCameraPath;
};

template <VCM::MISMode MISMode>
class IntDirect : public IIntegrator {
public:
	explicit IntDirect(const DiParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntDirect() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		const bool hasInfLights = !ctx->scene()->infiniteLights().empty();
		if (hasInfLights)
			return std::make_shared<IntDirectInstance<true, MISMode>>(mParameters, ctx->lightSampler());
		else
			return std::make_shared<IntDirectInstance<false, MISMode>>(mParameters, ctx->lightSampler());
	}

private:
	const DiParameters mParameters;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	explicit IntDirectFactory(const ParameterGroup& params)
	{
		mParameters.MaxCameraRayDepthHard = (size_t)params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", mParameters.MaxCameraRayDepthSoft));

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
			return std::make_shared<IntDirect<VCM::MM_Balance>>(mParameters);
		case VCM::MM_Power:
			return std::make_shared<IntDirect<VCM::MM_Power>>(mParameters);
		}
	}

private:
	DiParameters mParameters;
	VCM::MISMode mMISMode;
};

class IntDirectFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntDirectFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "direct", "standard", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntDirectFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)