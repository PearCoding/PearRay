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

inline static auto safeDiv(const SpectralBlob& a, const SpectralBlob& b)
{
	return (b.cwiseAbs() <= PR_EPSILON).select(SpectralBlob::Zero(), a / b);
}

struct DiParameters {
	size_t MaxCameraRayDepthHard = 64;
	size_t MaxCameraRayDepthSoft = 4;
	bool DoNEE					 = true;
	bool DoDirect				 = true;
};

/// Standard path tracing
/// Suports NEE, Inf Lights
/// TODO: Mediums/Volume
template <bool HasInfLights, VCM::MISMode MISMode, bool EmissiveScatter>
class IntDirectInstance : public IIntegratorInstance {
private:
	struct TraversalContext {
		SpectralBlob Throughput	   = SpectralBlob::Ones();
		bool LastWasDelta		   = true;
		bool LastWasFluorescent	   = false;
		bool LastWasEmissive	   = false;
		SpectralBlob PathPDF	   = SpectralBlob::Ones();
		SpectralBlob PrevPathPDF   = SpectralBlob::Ones();
		SpectralBlob WavelengthPDF = SpectralBlob::Zero(); // Wavelength of the initial sampling
		Vector3f LastPosition	   = Vector3f::Zero();	   // Used in NEE
		Vector3f LastNormal		   = Vector3f::Zero();
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

		const bool hasEmission = entity->hasEmission();
		if (mParameters.DoDirect && hasEmission) {
			handleDirectHit(session, ip, entity, current);
			if constexpr (!EmissiveScatter)
				return {};
		}

		// If there is no material to scatter from, give up
		if (PR_UNLIKELY(!material))
			return {};

		if (mParameters.DoNEE && !material->hasOnlyDeltaDistribution() && !hasEmission)
			handleNEE(session, ip, material, current);

		current.LastWasEmissive = hasEmission;
		return handleScattering(session, ip, material, current);
	}

	// First camera vertex
	void traceCameraPath(RenderTileSession& session, const IntersectionPoint& initial_hit,
						 const RayGroup& rayGroup, IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Initial camera vertex
		TraversalContext current;
		current.WavelengthPDF = rayGroup.WavelengthPDF;
		//current.Throughput /= current.WavelengthPDF[0];

		mCameraWalker.traverse(
			session, initial_hit, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(session, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				mCameraPath.addToken(LightPathToken::Background());
				if constexpr (HasInfLights) {
					if (mParameters.DoDirect)
						handleInfLights(session, current, ray);
					else
						handleZero(session, current, ray);
				} else {
					handleZero(session, current, ray);
				}
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
			RayGroup rayGroup;
			sg.extractRayGroup(i, rayGroup);
			traceCameraPath(session, spt, rayGroup, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
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
										IMaterial* material, TraversalContext& current)
	{
		auto& rnd = session.random(ip.Ray.PixelIndex);

		current.LastPosition = ip.P;
		current.LastNormal	 = ip.Surface.N;

		// Russian roulette
		const auto roulette = mCameraRR.check(rnd, ip.Ray.IterationDepth + 1, material && material->hasOnlyDeltaDistribution());
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

		const Vector3f L		   = sout.globalL(ip);
		current.LastWasDelta	   = sout.isDelta();
		current.LastWasFluorescent = sout.isFluorescent();
		//current.LastWasEmissive was updated in parent function
		current.PrevPathPDF = current.PathPDF;
		current.PathPDF *= sout.PDF_S * scatProb;

		if ((current.PathPDF <= PDF_EPS).all()) // Catch this case, or everything will explode
			return {};

		// Update throughput
		current.Throughput *= sout.IntegralWeight; // cosine term and pdf already applied inside material

		if (sout.isHeroCollapsing()) {
			current.Throughput *= SpectralBlobUtils::HeroOnly();
			current.PathPDF *= SpectralBlobUtils::HeroOnly();
		}

		if (current.Throughput.isZero(PR_EPSILON))
			return {};

		// Setup ray flags
		RayFlags rflags = RayFlag::Bounce;
		if (sout.isHeroCollapsing())
			rflags |= RayFlag::Monochrome;

		Ray nextRay = ip.nextRay(L, rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
		if (current.LastWasFluorescent)
			nextRay.WavelengthNM = sout.FluorescentWavelengthNM;

		return std::make_optional(nextRay);
	}

	/// Handle simple Next Event Estimation (aka, connect point with light)
	void handleNEE(RenderTileSession& session, const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, TraversalContext& current)
	{
		const EntitySamplingInfo sampleInfo = { cameraIP.P, cameraIP.Surface.N };

		// Sample light
		LightSampleInput lsin(session.random(cameraIP.Ray.PixelIndex));
		lsin.WavelengthNM	  = cameraIP.Ray.WavelengthNM;
		lsin.Point			  = &cameraIP;
		lsin.SamplingInfo	  = &sampleInfo;
		lsin.SamplePosition	  = true;
		lsin.SampleWavelength = cameraMaterial->hasFluorescence();
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, session);
		const Light* light = lsample.first;
		if (PR_UNLIKELY(!light))
			return;

		// Calculate geometry stuff
		const float sqrD = (lsout.LightPosition - cameraIP.P).squaredNorm();
		const Vector3f L = lsout.Outgoing;
		const float cosC = std::abs(L.dot(cameraIP.Surface.N));
		const float cosL = std::abs(lsout.CosLight);
		//const bool front	  = lsout.CosLight >= 0.0f;
		const bool isFeasible = cosC * cosL > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		if (!isFeasible) // MIS is zero
			return;

		// Evaluate camera material
		MaterialEvalInput min;
		min.Context							= MaterialEvalContext::fromIP(cameraIP, L);
		min.Context.FluorescentWavelengthNM = lsout.WavelengthNM;
		min.ShadingContext					= ShadingContext::fromIP(session.threadID(), cameraIP);
		MaterialEvalOutput mout;
		cameraMaterial->eval(min, mout, session);

		// Due to fancy material evaluation we might get a delta here
		if (PR_UNLIKELY(mout.isDelta()))
			return;

		// Each wavelength could have generated the path
		const bool rayMonochrome		 = cameraIP.Ray.isMonochrome();
		const bool bsdfMonochrome		 = mout.isHeroCollapsing() || rayMonochrome;
		const SpectralBlob rayHeroFactor = rayMonochrome ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		const SpectralBlob heroFactor	 = bsdfMonochrome ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		const SpectralBlob bsdfWvlPdfS	 = mout.PDF_S * heroFactor;

		// Its impossible to sample the bsdf, so skip it
		if ((bsdfWvlPdfS <= PDF_EPS).all())
			return;

		// Check if a check between point and light is necessary
		const SpectralBlob connectionW = lsout.Radiance * mout.Weight;
		const bool worthACheck		   = /*front &&*/ !connectionW.isZero(PR_EPSILON);

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
			lightPdfS *= lsample.second; /* Apply selection probability */
			if (!std::isnormal(lightPdfS) || lightPdfS <= PDF_EPS)
				return;
		}

		// lightPdfS is the sampled pdf which is only affected by the hero wavelength
		const SpectralBlob lightPdfS2 = lightPdfS * lsout.Wavelength_PDF * rayHeroFactor;
		if ((lightPdfS2 <= PDF_EPS).all())
			return;

		// Calculate MIS
		// Note, we omit the wavelength in the pdf which constructed the path,
		// but not in the hypothetical one, as the former cancels out in the end.
		SpectralBlob mis;
		if (mParameters.DoDirect && !current.LastWasEmissive) {
			const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
			const float cameraRoulette	  = mCameraRR.probability(cameraPathLength);

			const SpectralBlob bsdfPdfS = bsdfWvlPdfS * cameraRoulette;
			const float denom			= VCM::mis_term<MISMode>(current.PathPDF * lightPdfS2).sum() + VCM::mis_term<MISMode>(current.PathPDF * bsdfPdfS).sum();
			mis							= light->hasDeltaDistribution() ? (heroFactor / heroFactor.sum()).eval() : (VCM::mis_term<MISMode>(current.PathPDF[0] * lightPdfS2[0]) / (heroFactor * denom * VCM::mis_term<MISMode>(current.WavelengthPDF))).eval();

			//PR_ASSERT((mis <= PR_SPECTRAL_BLOB_SIZE).all(), "MIS must be between 0 and PR_SPECTRAL_BLOB_SIZE");
		} else {
			mis = heroFactor / (heroFactor.sum() * current.WavelengthPDF);
		}

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		Ray shadow			 = cameraIP.nextRay(L, RayFlag::Shadow, SHADOW_RAY_MIN, distance);
		shadow.WavelengthNM	 = min.Context.FluorescentWavelengthNM;
		const bool isVisible = worthACheck && !session.traceShadowRay(shadow, distance);

		// Calculate contribution (cosine term already applied inside material)
		const SpectralBlob contrib = isVisible ? (connectionW / lightPdfS2[0]).eval() : SpectralBlob::Zero();

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
									 shadow, mCameraPath);

		mCameraPath.popToken(2);
	}

	/// Handle case where camera ray directly hits emissive object
	void handleDirectHit(RenderTileSession& session, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, TraversalContext& current)
	{
		const EntitySamplingInfo sampleInfo = { current.LastPosition, current.LastNormal };

		const IEmission* ems = session.getEmission(cameraIP.Surface.Geometry.EmissionID);
		if (PR_UNLIKELY(!ems))
			return;

		// Cull emission if needed
		const float cosC = -cameraIP.Surface.NdotV;
		if (std::abs(cosC) <= PR_EPSILON)
			return;
		const bool hitFromBehind = cosC < 0.0f;

		SpectralBlob radiance;
		if (hitFromBehind) {
			radiance = SpectralBlob::Zero();
		} else {
			// Evaluate emission
			EmissionEvalInput ein;
			ein.Context		   = EmissionEvalContext::fromIP(cameraIP, -cameraIP.Ray.Direction);
			ein.ShadingContext = ShadingContext::fromIP(session.threadID(), cameraIP);
			EmissionEvalOutput eout;
			ems->eval(ein, eout, session);
			radiance = eout.Radiance;
		}

		const SpectralBlob heroFactor = cameraIP.Ray.isMonochrome() ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();

		// If the given contribution can not be determined by NEE as well, do not calculate MIS
		if (!mParameters.DoNEE || hitFromBehind || current.LastWasDelta) {
			mCameraPath.addToken(LightPathToken::Emissive());
			session.pushSpectralFragment(heroFactor / (heroFactor.sum() * current.WavelengthPDF), current.Throughput, radiance, cameraIP.Ray, mCameraPath);
			mCameraPath.popToken();
			return;
		}

		// Evaluate PDF
		const float selProb = mLightSampler->pdfEntitySelection(cameraEntity);
		auto posPDF			= mLightSampler->pdfPosition(cameraEntity, cameraIP.P, &sampleInfo);
		if (posPDF.IsArea)
			posPDF.Value = IS::toSolidAngle(posPDF.Value, cameraIP.Depth2, std::abs(cosC));
		const float posPDF_S	   = posPDF.Value * selProb;
		const SpectralBlob wvlProb = SpectralBlob::Ones();
		/*cameraIP.Ray.IterationDepth > 0 && current.LastWasFluorescent ? mLightSampler->pdfWavelength(cameraIP.Ray.WavelengthNM, cameraIP.P, cameraEntity)
																								   : SpectralBlob::Ones();*/

		const float denom	   = VCM::mis_term<MISMode>(current.PrevPathPDF * wvlProb * posPDF_S).sum() + VCM::mis_term<MISMode>(current.PathPDF).sum();
		const SpectralBlob mis = heroFactor * VCM::mis_term<MISMode>(current.PathPDF[0]) / (denom * VCM::mis_term<MISMode>(current.WavelengthPDF));

		//PR_ASSERT((mis <= PR_SPECTRAL_BLOB_SIZE).all(), "MIS must be between 0 and PR_SPECTRAL_BLOB_SIZE");

		// Splat
		mCameraPath.addToken(LightPathToken::Emissive());
		session.pushSpectralFragment(mis, current.Throughput, radiance, cameraIP.Ray, mCameraPath);
		mCameraPath.popToken();
	}

	/// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const RenderTileSession& session, TraversalContext& current, const Ray& ray) const
	{
		session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount);
		const SpectralBlob heroFactor = (ray.Flags & RayFlag::Monochrome) ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();

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

			const float selProb		   = mLightSampler->pdfLightSelection(light);
			const float pdf_S		   = lout.Direction_PDF_S * selProb;
			const SpectralBlob wvlProb = SpectralBlob::Ones();
			//ray.IterationDepth > 0 && current.LastWasFluorescent ? mLightSampler->pdfWavelength(ray.WavelengthNM, ray.Origin, light) : SpectralBlob::Ones();
			radiance += lout.Radiance;
			denom_mis += VCM::mis_term<MISMode>(current.PrevPathPDF * wvlProb * pdf_S).sum();
		}

		// If the given contribution can not be determined by NEE as well, do not calculate MIS
		if (!mParameters.DoNEE || current.LastWasDelta) {
			session.pushSpectralFragment(heroFactor / (heroFactor.sum() * current.WavelengthPDF), current.Throughput, radiance, ray, mCameraPath);
			return;
		}

		// Calculate MIS
		const float denom	   = VCM::mis_term<MISMode>(current.PathPDF).sum() + denom_mis;
		const SpectralBlob mis = heroFactor * VCM::mis_term<MISMode>(current.PathPDF[0]) / (denom * VCM::mis_term<MISMode>(current.WavelengthPDF));

		PR_ASSERT((mis <= PR_SPECTRAL_BLOB_SIZE).all(), "MIS must be between 0 and PR_SPECTRAL_BLOB_SIZE");

		// Splat
		session.pushSpectralFragment(mis, current.Throughput, radiance, ray, mCameraPath);
	}

	/// Handle case where camera ray hits nothing and there is no inf-lights
	inline void handleZero(const RenderTileSession& session, TraversalContext& current, const Ray& ray) const
	{
		session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount);
		const SpectralBlob heroFactor = (ray.Flags & RayFlag::Monochrome) ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		session.pushSpectralFragment(heroFactor / (heroFactor.sum() * current.WavelengthPDF), current.Throughput, SpectralBlob::Zero(), ray, mCameraPath);
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

		mParameters.DoNEE	 = params.getBool("nee", true);
		mParameters.DoDirect = params.getBool("direct", true);
		mEmissiveScatter	 = params.getBool("emissive_scatter", true);
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