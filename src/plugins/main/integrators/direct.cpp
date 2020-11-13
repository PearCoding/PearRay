#include "Environment.h"
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

#include "IntegratorUtils.h"
#include "Logger.h"
#include "Walker.h"

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

static inline float culling(float cos)
{
#ifdef PR_NO_CULLING
	return std::abs(cos);
#else
	return std::max(0.0f, cos);
#endif
}

struct DiParameters {
	size_t MaxCameraRayDepthHard = 64;
	size_t MaxCameraRayDepthSoft = 4;
	size_t MaxLightSamples		 = 1;
};

struct DiContribution {
	float MIS; // The system supports SpectralBlob MIS, but for path tracing intermediates float is sufficient
	SpectralBlob Importance;
	SpectralBlob Radiance;
};
using Contribution					   = DiContribution;
const static Contribution ZERO_CONTRIB = Contribution{ 0.0f, SpectralBlob::Zero(), SpectralBlob::Zero() };

using CameraPathWalker = Walker<true>; // Enable russian roulette
class IntDirectInstance : public IIntegratorInstance {
public:
	explicit IntDirectInstance(RenderContext* ctx, const DiParameters& parameters)
		: mSampler((parameters.MaxCameraRayDepthHard + 1) * (parameters.MaxLightSamples * 4 + 3))
		, mInfLightCount(ctx->scene()->infiniteLights().size())
		, mParameters(parameters)
		, mLightSampler(ctx->lightSampler())
	{
	}

	virtual ~IntDirectInstance() = default;

	// Do NEE
	Contribution sampleLight(RenderTileSession& session, const IntersectionPoint& spt,
							 LightPathToken& mat_token, LightPathToken& light_token,
							 IMaterial* material, bool handleMSI)
	{
		PR_PROFILE_THIS;

		const EntitySamplingInfo sampleInfo = { spt.P, spt.Surface.N };

		LightSampleInput lsin;
		lsin.RND			= mSampler.next4D();
		lsin.WavelengthNM	= spt.Ray.WavelengthNM;
		lsin.Point			= &spt;
		lsin.SamplingInfo	= &sampleInfo;
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, session);
		if (PR_UNLIKELY(!lsample.first))
			return ZERO_CONTRIB;

		// Sample light
		Vector3f L		 = (lsout.LightPosition - spt.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosO = culling(lsout.CosLight);
		float pdfS		 = lsout.Position_PDF.Value * lsample.second;
		if (lsout.Position_PDF.IsArea)
			pdfS = IS::toSolidAngle(pdfS, sqrD, cosO); // The whole integration is in solid angle domain -> Map into it

		const bool isFeasible = pdfS > PR_EPSILON && cosO > PR_EPSILON;

		L = lsout.Outgoing;
		if (lsample.first->isInfinite())
			light_token = LightPathToken::Background();
		else
			light_token = LightPathToken::Emissive();

		// Trace shadow ray
		const float NdotL	 = L.dot(spt.Surface.N);
		const float distance = lsample.first->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Vector3f oN	 = NdotL < 0 ? -spt.Surface.N : spt.Surface.N; // Offset normal used for safe positioning
		const Ray shadow	 = spt.Ray.next(spt.P, L, oN, RF_Shadow, SHADOW_RAY_MIN, distance);
		const bool isVisible = isFeasible && !session.traceShadowRay(shadow, distance, lsample.first->entityID());

		const SpectralBlob lightW = isVisible ? lsout.Radiance : SpectralBlob::Zero();

		// Evaluate surface
		MaterialEvalInput in{ MaterialEvalContext::fromIP(spt, L), ShadingContext::fromIP(session.threadID(), spt) };
		MaterialEvalOutput out;
		material->eval(in, out, session);
		mat_token = LightPathToken(out.Type);

		float mis = 1;
		if (handleMSI) {
			const float matPDF = /*(spt.Ray.Flags & RF_Monochrome) ?*/ out.PDF_S[0] /*: (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE)*/;
			mis				   = MIS(mParameters.MaxLightSamples, pdfS, 1, matPDF);
		}

		return Contribution{ mis, out.Weight / pdfS, lightW };
	}

	// Estimate indirect light
	bool scatterLight(RenderTileSession& session, LightPath& path,
					  const IntersectionPoint& spt, float mis, const SpectralBlob& importance,
					  IMaterial* material)
	{
		PR_PROFILE_THIS;

		const bool allowMIS = !material->hasDeltaDistribution();

		constexpr float DEPTH_DROP_RATE = 0.90f;
		constexpr float SCATTER_EPS		= 1e-4f;

		// Russian Roulette
		// TODO: Add adjoint russian roulette (and maybe splitting)
		const float roussian_prob = mSampler.next1D();
		const float scatProb	  = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.IterationDepth - (int)mParameters.MaxCameraRayDepthSoft));
		if (spt.Ray.IterationDepth + 1 >= mParameters.MaxCameraRayDepthHard
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
		if (session.traceSingleRay(next, npos, npt, nentity, nmaterial)) {
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
			if (nentity->hasEmission()
				&& culling(-spt2.Surface.NdotV) > PR_EPSILON // Check if frontside
				&& PR_LIKELY(spt2.Depth2 > PR_EPSILON)) {	 // Check if not too close
				IEmission* ems = session.getEmission(spt2.Surface.Geometry.EmissionID);
				if (PR_LIKELY(ems)) {
					// Evaluate light
					EmissionEvalInput inL;
					inL.Entity		   = nentity;
					inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt2);
					EmissionEvalOutput outL;
					ems->eval(inL, outL, session);

					const EntitySamplingInfo sampleInfo = { spt.P, spt.Surface.N };

					const float pdfSel	  = mLightSampler->pdfSelection(nentity);
					const auto pdfL		  = mLightSampler->pdfPosition(nentity, &sampleInfo);
					const float lightPdfA = pdfSel * (pdfL.IsArea ? IS::toSolidAngle(pdfL.Value, spt2.Depth2, aNdotV) : pdfL.Value);
					next_mis			  = allowMIS ? MIS(1, out.PDF_S[0], mParameters.MaxLightSamples, lightPdfA) : 1.0f;

					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob(next_mis * mis), weighted_importance, outL.Radiance, next, path);
					path.popToken();
				}
			}

			if (PR_LIKELY(nmaterial))
				evalN(session, path, spt2, mis * next_mis, weighted_importance, nentity, nmaterial);
		} else {
			path.addToken(LightPathToken::Background());
			IntegratorUtils::handleBackground(session, next, [&](const InfiniteLightEvalOutput& ileout) {
				hasScattered	   = true;
				const float matPDF = /*(spt.Ray.Flags & RF_Monochrome) ?*/ out.PDF_S[0] /*: (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE)*/;
				const float msiL   = allowMIS ? MIS(1, matPDF, mParameters.MaxLightSamples, ileout.Direction_PDF_S) : 1.0f;
				session.pushSpectralFragment(SpectralBlob(msiL * mis), weighted_importance, ileout.Radiance, next, path);
			});
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
		session.tile()->statistics().addCameraDepthCount();
#ifdef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Scatter Light
		bool hasScatterContrib = scatterLight(session, path, spt, mis, importance, material);

		if (material->hasDeltaDistribution())
			return;

		// Direct Light
		const float factor = 1.0f / mParameters.MaxLightSamples;
		for (size_t i = 0; i < mParameters.MaxLightSamples; ++i) {
			LightPathToken token1;
			LightPathToken token2;
			const auto contrib = sampleLight(session, spt, token1, token2, material, hasScatterContrib);

			if (contrib.MIS <= PR_EPSILON)
				continue;

			path.addToken(token1);
			path.addToken(token2);
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
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

#ifndef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Only consider camera rays, as everything else is handled eventually by MIS
		if (entity->hasEmission()
			&& culling(-spt.Surface.NdotV) > PR_EPSILON) // Check if frontside)
		{
			IEmission* ems = session.getEmission(spt.Surface.Geometry.EmissionID);
			if (PR_LIKELY(ems)) {
				// Evaluate light
				EmissionEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);
				EmissionEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Radiance.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), outL.Radiance, spt.Ray, path);
					path.popToken();
				}
			}
		}

		evalN(session, path, spt, 1, SpectralBlob::Ones(), entity, material);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const size_t maxPathSize = mParameters.MaxCameraRayDepthHard + 2;

		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());

		session.tile()->statistics().addCameraDepthCount(sg.size());
		session.tile()->statistics().addEntityHitCount(sg.size());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);
			mSampler.reset(session.random());

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
					IntegratorUtils::handleBackgroundGroup(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	SampleArray mSampler;
	const size_t mInfLightCount;
	const DiParameters mParameters;
	const std::shared_ptr<LightSampler> mLightSampler;
};

class IntDirect : public IIntegrator {
public:
	explicit IntDirect(const DiParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntDirect() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntDirectInstance>(ctx, mParameters);
	}

private:
	const DiParameters mParameters;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	explicit IntDirectFactory(const ParameterGroup& params)
	{
		mParameters.MaxLightSamples		  = (size_t)params.getUInt("light_sample_count", mParameters.MaxLightSamples);
		mParameters.MaxCameraRayDepthHard = (size_t)params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", mParameters.MaxCameraRayDepthSoft));
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		return std::make_shared<IntDirect>(mParameters);
	}

private:
	DiParameters mParameters;
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