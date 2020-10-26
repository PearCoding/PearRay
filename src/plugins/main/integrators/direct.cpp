#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "buffer/Feedback.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "path/LightPath.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "sampler/SampleArray.h"
#include "trace/IntersectionPoint.h"

#include "IntegratorUtils.h"
#include "Logger.h"

// Define this to let all rays regardless of depth contribute to SP AOVs, else only camera rays are considered
//#define PR_ALL_RAYS_CONTRIBUTE_SP

#define MIS(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

/* Based on Path Integral formulation: (here length k)
 * f(x) is the brdf. In our case a material already includes the cosine factor f(x)cos(N,x)
 *
 * L(x0->x1)G(x0<->x1)f(x1)/p(x0)
 * |*| f(x2)cos(N2,x1<-x2)/p_(x1|x2)
 * |*| ...
 * |*| f(x(k-1))cos(N(k-1),x(k-2)<-x(k-1))/p_(x(k-2)|x(k-1))
 * |*| W(xk)
 *
 * p(xi) is the pdf with respect to the area
 * p_(xi) is the pdf with respect to the solid angle (not projected solid angle!)
 */
namespace PR {
struct Contribution {
	float MIS; // The system supports SpectralBlob MIS, but for path tracing intermediates float is sufficient
	SpectralBlob Importance;
	SpectralBlob Radiance;
};
const static Contribution ZERO_CONTRIB = Contribution{ 0.0f, SpectralBlob::Zero(), SpectralBlob::Zero() };

constexpr float SHADOW_RAY_MIN = 0.0001f;
constexpr float SHADOW_RAY_MAX = PR_INF;
constexpr float BOUNCE_RAY_MIN = SHADOW_RAY_MIN;
constexpr float BOUNCE_RAY_MAX = PR_INF;

class IntDirectInstance : public IIntegratorInstance {
public:
	explicit IntDirectInstance(RenderContext* ctx, size_t lightSamples, size_t maxRayDepthSoft, size_t maxRayDepthHard, bool mis)
		: mSampler((maxRayDepthHard + 1) * (lightSamples * 3 + ctx->scene()->infiniteLights().size() * 2 + 3))
		, mInfLightCount(ctx->scene()->infiniteLights().size())
		, mLightSampleCount(lightSamples)
		, mMaxRayDepthSoft(maxRayDepthSoft)
		, mMaxRayDepthHard(maxRayDepthHard)
		, mMISEnabled(mis)
	{
		mInfLightHandled.resize(maxRayDepthHard * mInfLightCount, false);
	}

	virtual ~IntDirectInstance() = default;

	// Estimate direct (infinite) light
	Contribution infiniteLight(RenderTileSession& session, const IntersectionPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material, bool handleMSI)
	{
		PR_PROFILE_THIS;

		const bool allowMIS = mMISEnabled && handleMSI && !infLight->hasDeltaDistribution();

		// Sample light
		InfiniteLightSampleInput inL;
		inL.Point = spt;
		inL.RND	  = mSampler.next2D();
		InfiniteLightSampleOutput outL;
		infLight->sample(inL, outL, session);

		// An unlikely event, but a zero pdf has to be catched before blowing up other parts
		if (PR_UNLIKELY(outL.PDF_S <= PR_EPSILON))
			return ZERO_CONTRIB;

		if (infLight->hasDeltaDistribution())
			outL.PDF_S = 1;

		// Trace shadow ray
		const Ray shadow		= spt.Ray.next(spt.P, outL.Outgoing, spt.Surface.N, RF_Shadow, SHADOW_RAY_MIN, SHADOW_RAY_MAX);
		const bool hitSomething = session.traceOcclusionRay(shadow);

		SpectralBlob evalW;
		if (hitSomething) { // If we hit something before the light/background, the light path is occluded
			evalW = SpectralBlob::Zero();
		} else {
			evalW = outL.Radiance;
		}

		// Evaluate surface
		MaterialEvalInput in{ MaterialEvalContext::fromIP(spt, shadow.Direction), ShadingContext::fromIP(session.threadID(), spt) };
		MaterialEvalOutput out;
		material->eval(in, out, session);
		token = LightPathToken(out.Type);

		// Calculate hero mis pdf
		const float matPDF = (spt.Ray.Flags & RF_Monochrome) ? out.PDF_S[0] : (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE);
		const float msiL   = allowMIS ? MIS(1, outL.PDF_S, 1, matPDF) : 1.0f;
		return Contribution{ msiL, out.Weight / outL.PDF_S, evalW };
	}

	// Estimate direct (finite) light
	Contribution directLight(RenderTileSession& session, const IntersectionPoint& spt,
							 LightPathToken& token,
							 IMaterial* material, bool handleMSI)
	{
		PR_PROFILE_THIS;

		// Pick light and point
		EntitySamplePDF entityPdf;
		GeometryPoint lightPt;
		Vector3f lightPos;
		EntitySamplingInfo sampleInfo = { spt.P, spt.Surface.N };

		IEntity* light = session.sampleLight(sampleInfo, spt.Surface.Geometry.EntityID, mSampler.next3D(),
											 lightPos, lightPt, entityPdf);
		if (PR_UNLIKELY(!light))
			return ZERO_CONTRIB;

		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (PR_UNLIKELY(!ems)) {
			session.pushFeedbackFragment(OF_MissingEmission, spt.Ray);
			return ZERO_CONTRIB;
		}

		// Sample light
		Vector3f L		 = (lightPos - spt.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosO = std::max(0.0f, -L.dot(lightPt.N)); // Only frontside
		float pdfS		 = entityPdf.Value;
		if (entityPdf.IsArea)
			pdfS = IS::toSolidAngle(pdfS, sqrD, cosO); // The whole integration is in solid angle domain -> Map into it

		if (PR_UNLIKELY(pdfS <= PR_EPSILON))
			return ZERO_CONTRIB;

		// Trace shadow ray
		const float NdotL	 = L.dot(spt.Surface.N);
		const float distance = std::sqrt(sqrD);
		const Vector3f oN	 = NdotL < 0 ? -spt.Surface.N : spt.Surface.N; // Offset normal used for safe positioning
		const Ray shadow	 = spt.Ray.next(spt.P, L, oN, RF_Shadow, SHADOW_RAY_MIN, distance);
		const bool shadowHit = cosO > PR_EPSILON && session.traceShadowRay(shadow, distance, light->id());

		SpectralBlob lightW;
		if (!shadowHit) {
			lightW = SpectralBlob::Zero();
		} else {
			// Evaluate light
			LightEvalInput inL;
			inL.Entity		   = light;
			inL.ShadingContext = ShadingContext::fromIP(session.threadID(), IntersectionPoint::forSurface(shadow, lightPos, lightPt));
			LightEvalOutput outL;
			ems->eval(inL, outL, session);
			lightW = outL.Weight;
		}

		// Evaluate surface
		MaterialEvalInput in{ MaterialEvalContext::fromIP(spt, L), ShadingContext::fromIP(session.threadID(), spt) };
		MaterialEvalOutput out;
		material->eval(in, out, session);
		token = LightPathToken(out.Type);

		float mis = 1;
		if (mMISEnabled && handleMSI) {
			const float matPDF = (spt.Ray.Flags & RF_Monochrome) ? out.PDF_S[0] : (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE);
			mis				   = MIS(mLightSampleCount, pdfS, 1, matPDF);
		}

		return Contribution{ mis, out.Weight / pdfS, lightW };
	}

	// Estimate indirect light
	bool scatterLight(RenderTileSession& session, LightPath& path,
					  const IntersectionPoint& spt, float mis, const SpectralBlob& importance,
					  IMaterial* material)
	{
		PR_PROFILE_THIS;

		const bool allowMIS = mMISEnabled && !material->hasDeltaDistribution();

		constexpr float DEPTH_DROP_RATE = 0.90f;
		constexpr float SCATTER_EPS		= 1e-4f;

		// Russian Roulette
		// TODO: Add adjoint russian roulette (and maybe splitting)
		const float roussian_prob = mSampler.next1D();
		const float scatProb	  = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.IterationDepth - (int)mMaxRayDepthSoft));
		if (spt.Ray.IterationDepth + 1 >= mMaxRayDepthHard
			|| roussian_prob > scatProb
			|| scatProb <= SCATTER_EPS)
			return false;

		// Sample BxDF
		MaterialSampleInput in{ MaterialSampleContext::fromIP(spt), ShadingContext::fromIP(session.threadID(), spt), mSampler.next2D() };
		MaterialSampleOutput out;
		material->sample(in, out, session);

		if (PR_UNLIKELY(out.PDF_S[0] <= PR_EPSILON))
			return false;

		SpectralBlob next_importance = importance;

		// Construct next ray
		Ray next = spt.Ray.next(spt.P, out.globalL(spt), spt.Surface.N, RF_Bounce, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
		if (material->hasDeltaDistribution())
			next_importance /= scatProb;
		else
			next_importance /= (scatProb * out.PDF_S[0]); // The sampling is only done by the hero wavelength

		if (material->isSpectralVarying())
			next.Flags |= RF_Monochrome;

		path.addToken(LightPathToken(out.Type)); // (1)
		bool hasScattered = false;

		const SpectralBlob weighted_importance = next_importance * out.Weight;

		// Trace bounce ray
		GeometryPoint npt;
		Vector3f npos;
		IEntity* nentity = nullptr;
		IMaterial* nmaterial;
		if (session.traceBounceRay(next, npos, npt, nentity, nmaterial)) {
			IntersectionPoint spt2;
			spt2.setForSurface(next, npos, npt);
			const float aNdotV = std::abs(spt2.Surface.NdotV);

			// Giveup if bad
			if (PR_UNLIKELY(aNdotV <= PR_EPSILON)) {
				path.popToken();
				return false;
			}
			hasScattered = true;

			float next_mis = 1;

			// If we hit a light from the frontside, apply the lighting to current path (Emissive Term)
			if ((mMISEnabled || material->hasDeltaDistribution())
				&& nentity->isLight()
				&& next.Direction.dot(spt2.Surface.Geometry.N) < -PR_EPSILON // Check if frontside
				&& nentity->id() != spt.Surface.Geometry.EntityID			 // Check if not self
				&& PR_LIKELY(spt2.Depth2 > PR_EPSILON)) {					 // Check if not too close
				IEmission* ems = session.getEmission(spt2.Surface.Geometry.EmissionID);
				if (PR_LIKELY(ems)) {
					// Evaluate light
					LightEvalInput inL;
					inL.Entity		   = nentity;
					inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt2);
					LightEvalOutput outL;
					ems->eval(inL, outL, session);

					const auto pdfL = session.sampleLightPDF(EntitySamplingInfo{ spt.Surface.P, spt.Surface.N },
															 spt.Surface.Geometry.EntityID, nentity);
					next_mis		= allowMIS ? MIS(
									  1, out.PDF_S[0],
									  mLightSampleCount, pdfL.IsArea ? IS::toSolidAngle(pdfL.Value, spt2.Depth2, aNdotV) : pdfL.Value)
										: 1.0f;

					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob(next_mis * mis), weighted_importance, outL.Weight, next, path);
					path.popToken();
				}
			}

			if (PR_LIKELY(nmaterial))
				evalN(session, path, spt2, mis * next_mis, weighted_importance, nentity, nmaterial);
		} else {
			session.tile()->statistics().addBackgroundHitCount();

			// Theoretical question: If a non-delta light is hit, why try to sample them afterwards in evalN? -> Answer: You shall not!
			// A solution to prevent double count is to use a list of lights hit in the following loop
			// Afterwards in evalN only lights not on the list will be considered to be sampled from
			path.addToken(LightPathToken::Background());
			for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
				InfiniteLightEvalInput lin;
				lin.Point = &spt;
				lin.Ray	  = next;
				InfiniteLightEvalOutput lout;
				light->eval(lin, lout, session);

				if (lout.PDF_S <= PR_EPSILON)
					continue;

				mInfLightHandled[spt.Ray.IterationDepth * mInfLightCount + light->id()] = !allowMIS;
				hasScattered															= true;

				const float matPDF = (spt.Ray.Flags & RF_Monochrome) ? out.PDF_S[0] : (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE);
				const float msiL   = allowMIS ? MIS(1, matPDF, 1, lout.PDF_S) : 1.0f;
				session.pushSpectralFragment(SpectralBlob(msiL * mis), weighted_importance, lout.Radiance, next, path);
			}

			if (!hasScattered) // Make sure atleast the weights are updated
				session.pushSpectralFragment(SpectralBlob(mis), weighted_importance, SpectralBlob::Zero(), next, path);
			path.popToken();
		}

		path.popToken();
		return hasScattered;
	}

	// Every camera vertex
	void evalN(RenderTileSession& session, LightPath& path,
			   const IntersectionPoint& spt, float mis, const SpectralBlob& importance,
			   IEntity* entity, IMaterial* material)
	{
		PR_UNUSED(entity);
		// Early drop out for invalid splashes
		if (PR_UNLIKELY(!material))
			return;

		PR_PROFILE_THIS;

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addDepthCount();
#ifdef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Cleanup from previous calls
		for (size_t i = 0; i < mInfLightCount; ++i)
			mInfLightHandled[spt.Ray.IterationDepth * mInfLightCount + i] = false;

		// Scatter Light
		bool hasScatterContrib = scatterLight(session, path, spt, mis, importance, material);

		if (material->hasDeltaDistribution())
			return;

		// Direct Light
		const float factor = 1.0f / mLightSampleCount;
		for (size_t i = 0; i < mLightSampleCount; ++i) {
			LightPathToken token;
			const auto contrib = directLight(session, spt, token, material, hasScatterContrib);

			if (contrib.MIS <= PR_EPSILON)
				continue;

			path.addToken(token);
			path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
			session.pushSpectralFragment(SpectralBlob(contrib.MIS * mis), factor * importance * contrib.Importance, contrib.Radiance, spt.Ray, path);
			path.popToken(2);
		}

		// Infinite Lights
		// 1 sample per inf-light
		for (auto light : session.tile()->context()->scene()->infiniteLights()) {
			if (mInfLightHandled[spt.Ray.IterationDepth * mInfLightCount + light->id()]) // Skip if already handled
				continue;

			LightPathToken token;
			const auto contrib = infiniteLight(session, spt, token, light.get(), material, hasScatterContrib);

			if (contrib.MIS <= PR_EPSILON)
				continue;

			path.addToken(token);
			path.addToken(LightPathToken::Background());
			session.pushSpectralFragment(SpectralBlob(contrib.MIS * mis), factor * importance * contrib.Importance, contrib.Radiance, spt.Ray, path);
			path.popToken(2);
		}
	}

	// First camera vertex
	void eval0(RenderTileSession& session, LightPath& path,
			   const IntersectionPoint& spt,
			   IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Early drop out for invalid splashes
		if (!entity->isLight() && PR_UNLIKELY(!material))
			return;

#ifndef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Only consider camera rays, as everything else is handled eventually by MIS
		if (entity->isLight()) {
			IEmission* ems = session.getEmission(spt.Surface.Geometry.EmissionID);
			if (PR_LIKELY(ems)) {
				// Evaluate light
				LightEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);
				LightEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Weight.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), outL.Weight, spt.Ray, path);
					path.popToken();
				}
			}
		}

		evalN(session, path, spt, 1, SpectralBlob::Ones(), entity, material);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const size_t maxPathSize = mMaxRayDepthHard + 2;

		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());

		session.tile()->statistics().addDepthCount(sg.size());
		session.tile()->statistics().addEntityHitCount(sg.size());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);
			mSampler.reset(session.tile()->random());

			eval0(session, path, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
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
					IntegratorUtils::handleBackground(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	SampleArray mSampler;
	std::vector<bool> mInfLightHandled;
	const size_t mInfLightCount;
	const size_t mLightSampleCount;
	const size_t mMaxRayDepthSoft;
	const size_t mMaxRayDepthHard;
	const bool mMISEnabled;
};

class IntDirect : public IIntegrator {
public:
	explicit IntDirect(size_t lightSamples, size_t maxRayDepthSoft, size_t maxRayDepthHard, bool mis)
		: mLightSampleCount(lightSamples)
		, mMaxRayDepthSoft(maxRayDepthSoft)
		, mMaxRayDepthHard(maxRayDepthHard)
		, mMISEnabled(mis)
	{
	}

	virtual ~IntDirect() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntDirectInstance>(ctx, mLightSampleCount, mMaxRayDepthSoft, mMaxRayDepthHard, mMISEnabled);
	}

private:
	const size_t mLightSampleCount;
	const size_t mMaxRayDepthSoft;
	const size_t mMaxRayDepthHard;
	const bool mMISEnabled;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	explicit IntDirectFactory(const ParameterGroup& params)
	{
		mLightSampleCount = (size_t)params.getUInt("light_sample_count", 1);
		mMaxRayDepthHard  = (size_t)params.getUInt("max_ray_depth", 64);
		mMaxRayDepthSoft  = std::min(mMaxRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", 4));
		mMISEnabled		  = params.getBool("mis", true);
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		return std::make_shared<IntDirect>(mLightSampleCount, mMaxRayDepthSoft, mMaxRayDepthHard, mMISEnabled);
	}

private:
	size_t mLightSampleCount;
	size_t mMaxRayDepthSoft;
	size_t mMaxRayDepthHard;
	bool mMISEnabled;
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