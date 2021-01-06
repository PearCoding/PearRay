#include "Environment.h"
#include "IntegratorUtils.h"
#include "Logger.h"
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
#include "output/Feedback.h"
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
template <bool HasInfLights, VCM::MISMode MISMode, bool EmissiveScatter>
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

		session.tile()->statistics().add(RenderStatisticEntry::EntityHitCount);
		session.tile()->statistics().add(RenderStatisticEntry::CameraDepthCount);

		if (pathLength == 1)
			session.pushSPFragment(ip, mCameraPath);

		if (entity->hasEmission()) {
			handleDirectHit(session, ip, entity, current);
			if constexpr (!EmissiveScatter)
				return {};
		}

		// If there is no material to scatter from, give up
		if (PR_UNLIKELY(!material))
			return {};

		if (!material->hasOnlyDeltaDistribution())
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
				mCameraPath.addToken(LightPathToken::Background());
				if constexpr (HasInfLights)
					handleInfLights(session, current, ray);
				else
					handleZero(session, current, ray);
				mCameraPath.popToken();
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
	/// Handle scattering (aka, next ray direction)
	std::optional<Ray> handleScattering(RenderTileSession& session, const IntersectionPoint& ip,
										IEntity* entity, IMaterial* material, TraversalContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd = session.random();

		current.LastPosition = ip.P;
		current.LastNormal	 = ip.Surface.N;

		// Russian roulette
		const auto roulette = mCameraRR.check(session.random(), ip.Ray.IterationDepth + 1, material && material->hasOnlyDeltaDistribution());
		if (!roulette.has_value())
			return {};

		const float scatProb = roulette.value();

		// TODO: Add volume support
		if (!material)
			return {};

		// Sample Material
		MaterialSampleInput sin(rnd);
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		mCameraPath.addToken(sout.Type);

		const Vector3f L	 = sout.globalL(ip);
		const bool isDelta	 = sout.isDelta();
		current.LastWasDelta = isDelta;

		const float pdf_s = sout.PDF_S[0] * scatProb;
		current.LastPDF_S = pdf_s;

		if (pdf_s <= PDF_EPS) // Catch this case, or everything will explode
			return {};

		// Update throughput
		current.Throughput *= sout.Weight / pdf_s; // cosine term already applied inside material

		if (sout.isHeroCollapsing())
			current.Throughput *= SpectralBlobUtils::HeroOnly();

		if (current.Throughput.isZero(PR_EPSILON))
			return {};

		// Setup ray flags
		RayFlags rflags = RayFlag::Bounce;
		if (sout.isHeroCollapsing())
			rflags |= RayFlag::Monochrome;

		return std::make_optional(ip.nextRay(L, rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	/// Handle simple Next Event Estimation (aka, connect point with light)
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
		const float cosL	  = std::abs(lsout.CosLight);
		const bool front	  = lsout.CosLight >= 0.0f;
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

		if (mout.isDelta())
			return;

		const float bsdfWvlPdfS = (cameraIP.Ray.Flags & RayFlag::Monochrome) ? mout.PDF_S[0] : mout.PDF_S.sum(); // Each wavelength could have generated the path
		if (bsdfWvlPdfS <= PDF_EPS)																				 // Its impossible to sample the bsdf, so skip it
			return;

		// Check if a check between point and light is necessary
		const SpectralBlob connectionW = lsout.Radiance * mout.Weight;
		const bool worthACheck		   = front && !connectionW.isZero();

		// Evaluate direct pdf
		float lightPdfS = 0;
		if (light->hasDeltaDistribution()) {
			lightPdfS = 1;
		} else {
			if (light->isInfinite()) {
				lightPdfS = lsout.Direction_PDF_S;
			} else {
				// Convert position pdf to solid angle if necessary
				lightPdfS = lsout.Position_PDF.Value;
				if (lsout.Position_PDF.IsArea)
					lightPdfS = IS::toSolidAngle(lightPdfS, sqrD, cosL);
			}
			lightPdfS *= lsample.second;
			if (lightPdfS <= PDF_EPS)
				return;
		}

		// Calculate MIS
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const float cameraRoulette	  = mCameraRR.probability(cameraPathLength);

		const float bsdfPdfS = bsdfWvlPdfS * cameraRoulette;
		const float mis		 = light->hasDeltaDistribution() ? 1 : 1 / (1 + VCM::mis_term<MISMode>(bsdfPdfS / lightPdfS));

		if (mis <= MIS_EPS)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Ray shadow	 = cameraIP.nextRay(L, RayFlag::Shadow, SHADOW_RAY_MIN, distance);
		const bool isVisible = worthACheck && !session.traceShadowRay(shadow, distance);

		// Calculate contribution (cosinus term already applied inside material)
		const SpectralBlob contrib = isVisible ? (connectionW / lightPdfS).eval() : SpectralBlob::Zero();

		// Construct LPE path
		mCameraPath.addToken(mout.Type);

		if (light->isInfinite()) {
			session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount);
			mCameraPath.addToken(LightPathToken::Background());
		} else {
			session.tile()->statistics().add(RenderStatisticEntry::EntityHitCount);
			mCameraPath.addToken(LightPathToken::Emissive());
		}

		session.pushSpectralFragment(mis, current.Throughput, contrib,
									 cameraIP.Ray, mCameraPath);

		mCameraPath.popToken(2);
	}

	/// Handle case where camera ray directly hits emissive object
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
			mCameraPath.addToken(LightPathToken::Emissive());
			session.pushSpectralFragment(1, current.Throughput, radiance, cameraIP.Ray, mCameraPath);
			mCameraPath.popToken();
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

		// Note: No need to check LastPDF_S as handleScattering checks it already
		if (mis <= MIS_EPS)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		mCameraPath.addToken(LightPathToken::Emissive());
		session.pushSpectralFragment(mis, current.Throughput, radiance, cameraIP.Ray, mCameraPath);
		mCameraPath.popToken();
	}

	/// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const RenderTileSession& session, TraversalContext& current, const Ray& ray) const
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;
		session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount);

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
			session.pushSpectralFragment(1, current.Throughput, radiance, ray, mCameraPath);
			return;
		}

		// Calculate MIS
		const float mis = 1 / (1 + denom_mis);

		if (mis <= MIS_EPS)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		session.pushSpectralFragment(mis, current.Throughput, radiance, ray, mCameraPath);
	}

	/// Handle case where camera ray hits nothing and there is no inf-lights
	inline void handleZero(const RenderTileSession& session, TraversalContext& current, const Ray& ray) const
	{
		session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount);
		session.pushSpectralFragment(1, current.Throughput, SpectralBlob::Zero(), ray, mCameraPath);
	}

private:
	const DiParameters mParameters;
	const std::shared_ptr<LightSampler> mLightSampler;
	const RussianRoulette mCameraRR;
	const Walker mCameraWalker;

	LightPath mCameraPath;
};

template <VCM::MISMode MISMode, bool EmissiveScatter>
class IntDirect : public IIntegrator {
public:
	explicit IntDirect(const DiParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntDirect() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		const bool hasInfLights = ctx->scene()->infiniteLightCount() != 0;
		if (hasInfLights)
			return std::make_shared<IntDirectInstance<true, MISMode, EmissiveScatter>>(mParameters, ctx->lightSampler());
		else
			return std::make_shared<IntDirectInstance<false, MISMode, EmissiveScatter>>(mParameters, ctx->lightSampler());
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
			mMISMode = VCM::MISMode::Power;
		else
			mMISMode = VCM::MISMode::Balance;

		mEmissiveScatter = params.getBool("emissive_scatter", true);
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mMISMode) {
		default:
		case VCM::MISMode::Balance:
			if (mEmissiveScatter)
				return std::make_shared<IntDirect<VCM::MISMode::Balance, true>>(mParameters);
			else
				return std::make_shared<IntDirect<VCM::MISMode::Balance, false>>(mParameters);
		case VCM::MISMode::Power:
			if (mEmissiveScatter)
				return std::make_shared<IntDirect<VCM::MISMode::Power, true>>(mParameters);
			else
				return std::make_shared<IntDirect<VCM::MISMode::Power, false>>(mParameters);
		}
	}

private:
	DiParameters mParameters;
	VCM::MISMode mMISMode;
	bool mEmissiveScatter;
};

class IntDirectFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntDirectFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "direct", "standard", "default" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		const DiParameters parameters;
		return PluginSpecificationBuilder("PT", "Unidirectional Path Tracing")
			.Identifiers(getNames())
			.Inputs()
			.UInt("max_ray_depth", "Maximum ray depth allowed", parameters.MaxCameraRayDepthHard)
			.UInt("soft_max_ray_depth", "Maximum ray depth after which russian roulette tarts", parameters.MaxCameraRayDepthSoft)
			.Option("mis", "MIS mode", "balance", { "balance", "power" })
			.Bool("emissive_scatter", "Allow emissive surfaces to scatter", true)
			.Specification()
			.get();
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntDirectFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)