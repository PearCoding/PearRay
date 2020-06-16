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
#include "shader/ShadingPoint.h"

#include "Logger.h"

// Define this to let all rays regardless of depth contribute to SP AOVs, else only camera rays are considered
//#define PR_ALL_RAYS_CONTRIBUTE_SP

#define MSI(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

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
constexpr float SHADOW_RAY_MIN = 0.00001f;
constexpr float SHADOW_RAY_MAX = std::numeric_limits<float>::infinity();
constexpr float BOUNCE_RAY_MIN = SHADOW_RAY_MIN;
constexpr float BOUNCE_RAY_MAX = std::numeric_limits<float>::infinity();
class IntDirectInstance : public IIntegratorInstance {
public:
	explicit IntDirectInstance(RenderContext* ctx, size_t lightSamples, size_t maxRayDepth, bool msi)
		: mPipeline(ctx)
		, mLightSampleCount(lightSamples)
		, mMaxRayDepth(maxRayDepth)
		, mMSIEnabled(msi)
	{
	}

	virtual ~IntDirectInstance() = default;

	// Estimate direct (infinite) light
	SpectralBlob infiniteLight(RenderTileSession& session, const ShadingPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material)
	{
		PR_PROFILE_THIS;

		const bool allowMSI = mMSIEnabled && !infLight->hasDeltaDistribution();

		// (1) Sample light
		{
			InfiniteLightSampleInput inL;
			inL.Point = spt;
			inL.RND	  = session.tile()->random().get2D();
			InfiniteLightSampleOutput outL;
			infLight->sample(inL, outL, session);

			if (infLight->hasDeltaDistribution())
				outL.PDF_S = 1;

			if (outL.PDF_S > PR_EPSILON) {
				// Trace shadow ray
				const Ray shadow = spt.Ray.next(spt.P, outL.Outgoing, spt.N, RF_Shadow, SHADOW_RAY_MIN, SHADOW_RAY_MAX);
				bool hitAnything = session.traceOcclusionRay(shadow);
				if (!hitAnything) {
					// Evaluate surface
					MaterialEvalInput in;
					in.Point	= spt;
					in.Outgoing = outL.Outgoing;
					in.NdotL	= outL.Outgoing.dot(spt.N);
					MaterialEvalOutput out;
					material->eval(in, out, session);
					token = LightPathToken(out.Type);

					const float sum_pdfs = out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE;
					const float msiL	 = allowMSI ? MSI(1, outL.PDF_S, 1, sum_pdfs) : 1.0f;
					return msiL * outL.Weight * out.Weight / outL.PDF_S;
				}
			}
		}
		return SpectralBlob::Zero();
	}

	// Estimate direct (finite) light
	SpectralBlob directLight(RenderTileSession& session, const ShadingPoint& spt,
							 LightPathToken& token,
							 IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Pick light and point
		float pdfA;
		GeometryPoint lightPt;
		IEntity* light = session.pickRandomLight(spt.N, lightPt, pdfA);
		if (PR_UNLIKELY(!light))
			return SpectralBlob::Zero();

		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (PR_UNLIKELY(!ems)) {
			session.pushFeedbackFragment(OF_MissingEmission, spt.Ray);
			return SpectralBlob::Zero();
		}

		// Sample light
		Vector3f L		 = (lightPt.P - spt.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosO  = std::max(0.0f, -L.dot(lightPt.N));
		const float cosI  = std::max(0.0f, L.dot(spt.N));
		const float cgeom = cosO / sqrD; // Geometric term without cosI (as it is included in the material evaluation)
		const float C	  = cgeom / pdfA;

		if (cosI <= PR_EPSILON || C <= PR_EPSILON || pdfA <= PR_EPSILON || sqrD <= PR_EPSILON)
			return SpectralBlob::Zero();

		// Trace shadow ray
		const float distance = std::sqrt(sqrD);
		const Ray shadow	 = spt.Ray.next(spt.P, L, spt.N, RF_Shadow, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
		bool shadowHit		 = session.traceShadowRay(shadow, distance, light->id());

		if (!shadowHit)
			return SpectralBlob::Zero();

		// Evaluate light
		LightEvalInput inL;
		inL.Entity = light;
		inL.Point.setByIdentity(shadow, lightPt);
		LightEvalOutput outL;
		ems->eval(inL, outL, session);

		// Evaluate surface
		MaterialEvalInput in;
		in.Point	= spt;
		in.Outgoing = L;
		in.NdotL	= cosI;
		MaterialEvalOutput out;
		material->eval(in, out, session);

		if (mMSIEnabled) {
			const float pdfS	 = IS::toSolidAngle(pdfA, sqrD, cosO);
			const float sum_pdfs = out.PDF_S.sum();
			const float msiL	 = MSI(mLightSampleCount, pdfS, 1, sum_pdfs);
			return outL.Weight * out.Weight * (C * msiL);
		} else {
			return outL.Weight * out.Weight * C;
		}
	}

	// Estimate indirect light
	void scatterLight(RenderTileSession& session, LightPath& path,
					  const ShadingPoint& spt,
					  IMaterial* material,
					  bool& infLightHandled)
	{
		PR_PROFILE_THIS;

		infLightHandled		= false;
		const bool allowMSI = mMSIEnabled && !material->hasDeltaDistribution();

		constexpr int MIN_DEPTH			= 4; //Minimum level of depth after which russian roulette will apply
		constexpr float DEPTH_DROP_RATE = 0.90f;

		const float scatProb = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.IterationDepth - MIN_DEPTH));

		// Sample BxDF
		MaterialSampleInput in;
		in.Point = spt;
		in.RND	 = session.tile()->random().get2D();
		MaterialSampleOutput out;
		material->sample(in, out, session);

		// Russian Roulette
		if (PR_UNLIKELY(out.PDF_S[0] <= PR_EPSILON)
			|| spt.Ray.IterationDepth + 1 >= mMaxRayDepth
			|| session.tile()->random().getFloat() > scatProb)
			return;

		// Construct next ray
		Ray next = spt.Ray.next(spt.P, out.Outgoing, spt.N, RF_Bounce, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
		if (material->hasDeltaDistribution())
			next.Weight *= out.Weight / scatProb;
		else
			next.Weight *= out.Weight / (scatProb * out.PDF_S[0]); // The sampling is only done by the hero wavelength

		if (material->isSpectralVarying())
			next.Flags |= RF_Monochrome;

		if (PR_UNLIKELY(next.Weight.isZero()))
			return;

		path.addToken(LightPathToken(out.Type)); // (1)

		// Trace bounce ray
		GeometryPoint npt;
		IEntity* nentity;
		IMaterial* nmaterial;
		if (session.traceBounceRay(next, npt, nentity, nmaterial)) {
			ShadingPoint spt2;
			spt2.setByIdentity(next, npt);
			const float aNdotV = std::max(0.0f, -spt2.NdotV);

			// Giveup if bad
			if (PR_UNLIKELY(aNdotV <= PR_EPSILON || spt2.Depth2 <= PR_EPSILON)) {
				path.popToken();
				return;
			}

			// If we hit a light, apply the lighting to current path (Emissive Term)
			if (nentity->isLight()) {
				IEmission* ems = session.getEmission(spt2.Geometry.EmissionID);
				if (PR_LIKELY(ems)) {
					// Evaluate light
					LightEvalInput inL;
					inL.Entity = nentity;
					inL.Point  = spt2;
					LightEvalOutput outL;
					ems->eval(inL, outL, session);

					const float msiL = allowMSI
										   ? MSI(
											   1, out.PDF_S[0],
											   mLightSampleCount, IS::toSolidAngle(session.pickRandomLightPDF(next.Direction, nentity), spt2.Depth2, aNdotV))
										   : 1.0f;
					SpectralBlob radiance = msiL * outL.Weight * aNdotV / spt2.Depth2; // cos(NxL)/pdf(...) is already inside next.Weight

					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(radiance, next, path);
					path.popToken();
				}
			}

			evalN(session, path, spt2, nentity, nmaterial);
		} else {
			session.tile()->statistics().addBackgroundHitCount();
			infLightHandled = true;

			path.addToken(LightPathToken::Background());
			for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
				// Only the ray is fixed (Should be handled better!)
				InfiniteLightEvalInput lin;
				lin.Point.Ray	= next;
				lin.Point.Flags = SPF_Background;
				InfiniteLightEvalOutput lout;
				light->eval(lin, lout, session);

				const float msiL = allowMSI ? MSI(1, out.PDF_S[0], 1, lout.PDF_S) : 1.0f;
				session.pushSpectralFragment(lout.Weight * msiL, next, path);
			}
			path.popToken();
		}

		path.popToken();
	}

	// Every camera vertex
	void evalN(RenderTileSession& session, LightPath& path,
			   const ShadingPoint& spt,
			   IEntity* entity, IMaterial* material)
	{
		// Early drop out for invalid splashes
		if (PR_UNLIKELY(!material))
			return;

		PR_PROFILE_THIS;

#ifdef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Scatter Light
		bool infLightHandled;
		scatterLight(session, path, spt, material, infLightHandled);

		if (material->hasDeltaDistribution())
			return;

		// Direct Light
		const float factor = 1.0f / mLightSampleCount;
		for (size_t i = 0; i < mLightSampleCount; ++i) {
			LightPathToken token;
			SpectralBlob radiance = factor * directLight(session, spt, token, material);

			if (!radiance.isZero()) {
				path.addToken(token);
				path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
				session.pushSpectralFragment(radiance, spt.Ray, path);
				path.popToken(2);
			}
		}

		// Infinite Lights
		for (auto light : // Already handled in scatter equation
			 infLightHandled ? session.tile()->context()->scene()->deltaInfiniteLights()
							 : session.tile()->context()->scene()->infiniteLights()) {
			LightPathToken token;
			SpectralBlob radiance = infiniteLight(session, spt, token,
												  light.get(), material);

			if (!radiance.isZero()) {
				path.addToken(token);
				path.addToken(LightPathToken::Background());
				session.pushSpectralFragment(radiance, spt.Ray, path);
				path.popToken(2);
			}
		}
	}

	// First camera vertex
	void eval0(RenderTileSession& session, LightPath& path,
			   const ShadingPoint& spt,
			   IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		session.tile()->statistics().addEntityHitCount();

		// Early drop out for invalid splashes
		if (!entity->isLight() && !material)
			return;

#ifndef PR_ALL_RAYS_CONTRIBUTE_SP
		session.pushSPFragment(spt, path);
#endif

		// Only consider camera rays, as everything else produces too much noise
		if (entity->isLight()
			&& spt.Ray.IterationDepth == 0
			&& spt.Depth2 > PR_EPSILON) {
			IEmission* ems = session.getEmission(spt.Geometry.EmissionID);
			if (ems) {
				// Evaluate light
				LightEvalInput inL;
				inL.Entity = entity;
				inL.Point  = spt;
				LightEvalOutput outL;
				ems->eval(inL, outL, session);

				const float prevGEOM  = /*std::abs(spt.NdotV) * std::abs(spt.Ray.NdotL)*/ 1 /* spt.Depth2*/;
				SpectralBlob radiance = outL.Weight * prevGEOM;

				if (!radiance.isZero()) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(radiance, spt.Ray, path);
					path.popToken();
				}
			}
		}

		evalN(session, path, spt, entity, material);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const size_t maxPathSize = mMaxRayDepth + 2;

		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());

		for (size_t i = 0; i < sg.size(); ++i) {
			ShadingPoint spt;
			sg.computeShadingPoint(i, spt);

			eval0(session, path, spt, sg.entity(), session.getMaterial(spt.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
		}
	}

	void handleBackground(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		LightPath cb = LightPath::createCB();
		session.tile()->statistics().addBackgroundHitCount(sg.size());
		for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
			// Only the ray is fixed (Should be handled better!)
			for (size_t i = 0; i < sg.size(); ++i) {
				InfiniteLightEvalInput in;
				sg.extractRay(i, in.Point.Ray);
				in.Point.Flags = SPF_Background;
				InfiniteLightEvalOutput out;
				light->eval(in, out, session);

				session.pushSpectralFragment(out.Weight, in.Point.Ray, cb);
			}
		}
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;
		mPipeline.reset(session.tile());

		while (!mPipeline.isFinished()) {
			mPipeline.runPipeline();
			while (mPipeline.hasShadingGroup()) {
				auto sg = mPipeline.popShadingGroup(session);
				if (sg.isBackground())
					handleBackground(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	StreamPipeline mPipeline;
	const size_t mLightSampleCount;
	const size_t mMaxRayDepth;
	const bool mMSIEnabled;
};

class IntDirect : public IIntegrator {
public:
	explicit IntDirect(size_t lightSamples, size_t maxRayDepth, bool msi)
		: mLightSampleCount(lightSamples)
		, mMaxRayDepth(maxRayDepth)
		, mMSIEnabled(msi)
	{
	}

	virtual ~IntDirect() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntDirectInstance>(ctx, mLightSampleCount, mMaxRayDepth, mMSIEnabled);
	}

private:
	const size_t mLightSampleCount;
	const size_t mMaxRayDepth;
	const bool mMSIEnabled;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	explicit IntDirectFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		size_t lightsamples = (size_t)mParams.getUInt("light_sample_count", 1);
		size_t maxraydepth	= (size_t)mParams.getUInt("max_ray_depth", 4);

		auto obj = std::make_shared<IntDirect>(lightsamples, maxraydepth, mParams.getBool("msi", true));
		return obj;
	}

private:
	ParameterGroup mParams;
};

class IntDirectFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntDirectFactory>(ctx.Parameters);
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