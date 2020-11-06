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
#include "Walker.h"

#include "Logger.h"

/* Implementation of a bidirectional path tracer */

namespace PR {
struct BiDiParameters {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
};

struct BiDiPathVertex {
	IntersectionPoint IP;
	const IEntity* Entity;
	const IMaterial* Material;
	SpectralBlob Alpha; // Alpha term for next vertex
	float ForwardPDF_A;
	float BackwardPDF_A;
};

enum BiDiMISMode {
	BM_Balance,
	BM_Power
};

using LightPathWalker  = Walker<true>; // Enable russian roulette
using CameraPathWalker = Walker<true>; // Enable russian roulette

/// Bidirectional path tracer
/// This integrator is not optimized for speed, as generating a camera vertex list could be slow
/// However, instead of the recursive approach, it resembles the content of the original Veach chapter 10,
template <BiDiMISMode MISMode>
class IntBiDiInstance : public IIntegratorInstance {
public:
	explicit IntBiDiInstance(RenderContext* ctx, const BiDiParameters& parameters)
		: mParameters(parameters)
		, mLightSampler(ctx->lightSampler())
	{
		mLightPathWalker.MaxRayDepthHard  = mParameters.MaxLightRayDepthHard;
		mLightPathWalker.MaxRayDepthSoft  = mParameters.MaxLightRayDepthSoft;
		mCameraPathWalker.MaxRayDepthHard = mParameters.MaxCameraRayDepthHard;
		mCameraPathWalker.MaxRayDepthSoft = mParameters.MaxCameraRayDepthSoft;

		// Skip entry 0
		mLightVertices.reserve(mParameters.MaxLightRayDepthHard);
		mCameraVertices.reserve(mParameters.MaxCameraRayDepthHard);
	}

	virtual ~IntBiDiInstance() = default;

	/// Apply the MIS function to the given term
	inline static float mis_term(float a)
	{
		if constexpr (MISMode == BM_Power)
			return a * a;
		else
			return a;
	}

	/////////////////// Camera Path
	std::optional<Ray> handleCameraVertex(RenderTileSession& session, LightPath& path, const IntersectionPoint& ip,
										  IEntity* entity, IMaterial* material, float& nextPDF)
	{
		if (!entity || !material) // Nothing found, abort
			return {};

		auto& rnd = session.tile()->random();

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addDepthCount();

		const auto sc = ShadingContext::fromIP(session.threadID(), ip);

		const SpectralBlob prevAlpha = (ip.Ray.IterationDepth == 0) ? SpectralBlob::Ones() : mCameraVertices[ip.Ray.IterationDepth - 1].Alpha;
		const float prevPDF_A		 = (ip.Ray.IterationDepth == 0) ? 1 : mCameraVertices[ip.Ray.IterationDepth - 1].ForwardPDF_A;

		// Add pdf which generated this vertex
		float pdf_A = prevPDF_A;
		if (ip.Ray.IterationDepth > 0
			&& mCameraVertices[ip.Ray.IterationDepth - 1].Material
			&& !mCameraVertices[ip.Ray.IterationDepth - 1].Material->hasDeltaDistribution()) {
			pdf_A *= IS::toArea(nextPDF, ip.Depth2, std::abs(ip.Surface.NdotV));
		}

		// Handle C_0t (s==0)
		if (entity->hasEmission()) {
			IEmission* ems = session.getEmission(ip.Surface.Geometry.EmissionID);
			if (PR_LIKELY(ems)) {
				// Evaluate light
				EmissionEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = sc;
				EmissionEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Radiance.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE)); // TODO MIS
					session.pushSpectralFragment(SpectralBlob::Ones(), prevAlpha, outL.Radiance, ip.Ray, path);
					path.popToken();
				}
			}
		}

		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = sc;
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		if (!material->hasDeltaDistribution()) {
			sout.Weight /= sout.PDF_S[0];
			nextPDF = sout.PDF_S[0];
		} else {
			nextPDF = 1;
		}

		SpectralBlob alphaM = sout.Weight * prevAlpha;

		// Russian roulette
		if (ip.Ray.IterationDepth >= mParameters.MaxCameraRayDepthSoft) {
			constexpr float SCATTER_EPS = 1e-4f;

			const float russian_prob = rnd.getFloat();
			const float scatProb	 = std::min<float>(1.0f, std::pow(0.8f, ip.Ray.IterationDepth - mParameters.MaxCameraRayDepthSoft));
			if (russian_prob > scatProb || scatProb <= SCATTER_EPS)
				return {};

			alphaM /= scatProb;
		}

		if (material->isSpectralVarying())
			alphaM *= SpectralBlobUtils::HeroOnly();

		if (alphaM.isZero(PR_EPSILON) || PR_UNLIKELY(sout.PDF_S[0] <= PR_EPSILON))
			return {};

		path.addToken(sout.Type);
		mCameraVertices.emplace_back(BiDiPathVertex{ ip, entity, material, alphaM, pdf_A, pdf_A }); // TODO

		int rflags = RF_Bounce;
		if (material->isSpectralVarying())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.Ray.next(ip.P, sout.globalL(ip), ip.Surface.N,
											  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	// First camera vertex
	void handleCameraPath(RenderTileSession& session, LightPath& path,
						  const IntersectionPoint& spt,
						  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Early drop out for invalid splashes
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

		session.pushSPFragment(spt, path);

		// Intiial camera vertex
		mCameraVertices.emplace_back(BiDiPathVertex{ IntersectionPoint(), nullptr, nullptr, SpectralBlob::Ones(), 1, 1 });

		float nextPDF = 1.0f;
		mCameraPathWalker.traverse(
			session, spt, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(session, path, ip, entity2, material2, nextPDF);
			},
			[&](const Ray&) {
				// TODO: Do (s==0) if infinite lights are present
			});
	}

	/////////////////// Light Path
	std::optional<Ray> handleLightVertex(RenderTileSession& session, LightPath& path, const IntersectionPoint& ip,
										 IEntity* entity, IMaterial* material,
										 const SpectralBlob& lightRadiance, const LightPDF& lightPDF, float& nextPDF)
	{
		if (!entity || !material) // Nothing found, abort
			return {};

		auto& rnd = session.tile()->random();

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addDepthCount();

		const auto sc = ShadingContext::fromIP(session.threadID(), ip);

		const SpectralBlob prevWeight = (ip.Ray.IterationDepth == 0) ? lightRadiance : mLightVertices[ip.Ray.IterationDepth - 1].Alpha;
		const float prevPDF_A		  = (ip.Ray.IterationDepth == 0) ? 1 : mLightVertices[ip.Ray.IterationDepth - 1].ForwardPDF_A;

		// Add pdf which generated this vertex
		float pdf_A = prevPDF_A;
		if (ip.Ray.IterationDepth == 0) {
			if (!lightPDF.IsArea) // Convert to area
				pdf_A *= IS::toArea(lightPDF.Value, ip.Depth2, std::abs(ip.Surface.NdotV));
			else
				pdf_A *= lightPDF.Value;
		} else if (mLightVertices[ip.Ray.IterationDepth - 1].Material
				   && !mLightVertices[ip.Ray.IterationDepth - 1].Material->hasDeltaDistribution()) {
			pdf_A *= IS::toArea(nextPDF, ip.Depth2, std::abs(ip.Surface.NdotV));
		}

		// Our implementation does not support direct camera hits (no C_s0)
		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = sc;
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		if (!material->hasDeltaDistribution()) {
			sout.Weight /= sout.PDF_S[0];
			nextPDF = sout.PDF_S[0];
		} else {
			nextPDF = 1;
		}

		const float sn		= IntegratorUtils::correctShadingNormalForLight(-ip.Ray.Direction, sout.L, ip.Surface.N, ip.Surface.Geometry.N);
		SpectralBlob alphaM = sn * sout.Weight * prevWeight;

		// Russian roulette
		if (ip.Ray.IterationDepth >= mParameters.MaxLightRayDepthSoft) {
			constexpr float SCATTER_EPS = 1e-4f;

			const float russian_prob = rnd.getFloat();
			const float scatProb	 = std::min<float>(1.0f, std::pow(0.8f, ip.Ray.IterationDepth - mParameters.MaxLightRayDepthSoft));
			if (russian_prob > scatProb || scatProb <= SCATTER_EPS)
				return {};

			alphaM /= scatProb;
		}

		if (material->isSpectralVarying())
			alphaM *= SpectralBlobUtils::HeroOnly();

		if (alphaM.isZero(PR_EPSILON) || PR_UNLIKELY(sout.PDF_S[0] <= PR_EPSILON))
			return {};

		path.addToken(sout.Type);
		mLightVertices.emplace_back(BiDiPathVertex{ ip, entity, material, alphaM, pdf_A, pdf_A }); // TODO

		int rflags = RF_Bounce;
		if (material->isSpectralVarying())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.Ray.next(ip.P, sout.globalL(ip), ip.Surface.N,
											  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	// First camera vertex
	const Light* handleLightPath(RenderTileSession& session, LightPath& path,
								 const IntersectionPoint& spt)
	{
		PR_PROFILE_THIS;

		LightSampleInput lsin;
		lsin.WavelengthNM	= spt.Ray.WavelengthNM;
		lsin.RND			= session.tile()->random().get4D();
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const Light* light = mLightSampler->sample(lsin, lsout, session);
		if (!light)
			return light;

		Ray ray = Ray(lsout.LightPosition, -lsout.Outgoing);
		ray.Flags |= RF_Light;
		ray.WavelengthNM = lsin.WavelengthNM;

		LightPDF pdf = lsout.Position_PDF;
		pdf.Value *= lsout.Direction_PDF_S;
		pdf.Value					= std::isinf(pdf.Value) ? 1 : pdf.Value;
		const SpectralBlob radiance = lsout.Radiance / pdf.Value;

		if (light->isInfinite())
			path.addToken(LightPathToken::Background());
		else
			path.addToken(LightPathToken::Emissive());

		// Intiial light vertex
		mLightVertices.emplace_back(BiDiPathVertex{ IntersectionPoint(), nullptr, nullptr, SpectralBlob::Ones(), 1, 1 });

		float nextPDF = 1.0f;
		mLightPathWalker.traverse(
			session, ray,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleLightVertex(session, path, ip, entity2, material2, radiance, pdf, nextPDF);
			},
			[&](const Ray&) {
				// Do nothing! (as we do not support C_s0 connections)
			});

		return light;
	}

	///////////////////////////////////////
	void handleConnection(RenderTileSession& session, LightPath& fullPath,
						  size_t t, const LightPath& cameraPath,
						  size_t s, const LightPath& lightPath, const Light* light)
	{
		PR_ASSERT(t > 1 && s > 0, "Expected valid connection numbers");

		constexpr float DISTANCE_EPS = 0.000001f;

		const auto& cv	= mCameraVertices[t];
		const auto& lv	= mLightVertices[s];
		const auto& pcv = mCameraVertices[t - 1];
		const auto& plv = mLightVertices[s - 1];

		PR_UNUSED(light);
		/*if (s == 1) { // NEE
			// TODO
			PR_UNUSED(light);
		} else {*/
		// Check visibility

		Vector3f cD		  = (lv.IP.P - cv.IP.P); // Camera Vertex -> Light Vertex
		const float dist2 = cD.squaredNorm();
		if (dist2 <= DISTANCE_EPS)
			return; // Giveup as it is too close

		const float dist = std::sqrt(dist2);
		cD.normalize();

		const float cosC	 = cD.dot(cv.IP.Surface.N);
		const float cosL	 = -cD.dot(lv.IP.Surface.N);
		const Vector3f oN	 = cosC < 0 ? -cv.IP.Surface.N : cv.IP.Surface.N; // Offset normal used for safe positioning
		const Ray shadow	 = cv.IP.Ray.next(cv.IP.P, cD, oN, RF_Shadow, SHADOW_RAY_MIN, dist);
		const bool shadowHit = cosL > PR_EPSILON && session.traceShadowRay(shadow, dist, lv.Entity->id());

		// Extract visible and geometry term
		const bool isVisible = !shadowHit;
		const float Geometry = std::abs(cosC * cosL) / dist2;

		SpectralBlob lightW;
		SpectralBlob cameraW;
		if (isVisible) {
			// Evaluate light material
			MaterialEvalInput lin;
			lin.Context		   = MaterialEvalContext::fromIP(lv.IP, -cD);
			lin.ShadingContext = ShadingContext::fromIP(session.threadID(), lv.IP);
			MaterialEvalOutput lout;
			lv.Material->eval(lin, lout, session);
			lightW = lout.Weight;

			// Evaluate camera material
			MaterialEvalInput cin;
			cin.Context		   = MaterialEvalContext::fromIP(cv.IP, cD);
			cin.ShadingContext = ShadingContext::fromIP(session.threadID(), cv.IP);
			MaterialEvalOutput cout;
			cv.Material->eval(cin, cout, session);
			cameraW = cout.Weight;
		} else {
			// Do not evaluate if it is not visible
			lightW	= SpectralBlob::Zero();
			cameraW = SpectralBlob::Zero();
		}

		// Extract terms
		const SpectralBlob alphaC  = pcv.Alpha;
		const SpectralBlob alphaL  = plv.Alpha;
		const SpectralBlob weight  = alphaC * alphaL;
		const SpectralBlob contrib = lightW * Geometry * cameraW;

		// Compute MIS
		const float pdfT  = pcv.ForwardPDF_A;
		const float pdfS  = plv.ForwardPDF_A;
		const float pdfST = pdfS * pdfT; // Probability for generating this path
		float misDenom	  = 0.0f;

		//for(size_t i = 0; i < )

		const float mis = misDenom <= PR_EPSILON ? 1.0f : mis_term(pdfST) / misDenom;

		// Construct LPE path
		fullPath.reset();
		for (size_t t2 = 0; t2 < t; ++t2)
			fullPath.addToken(cameraPath.token(t2));
		for (size_t s2 = 0; s2 < s; ++s2)
			fullPath.addToken(lightPath.token(s - 1 - s2));

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), weight, contrib, cv.IP.Ray, fullPath);
		//}
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		LightPath fullPath(mParameters.MaxCameraRayDepthHard + mParameters.MaxLightRayDepthHard + 2);

		LightPath cameraPath(mParameters.MaxCameraRayDepthHard + 2);
		cameraPath.addToken(LightPathToken::Camera());

		LightPath lightPath(mParameters.MaxLightRayDepthHard + 2);

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			// Trace necessary paths
			const Light* light = handleLightPath(session, lightPath, spt);
			if (!light)
				return; // Giveup as no light is present
			PR_ASSERT(lightPath.currentSize() == mLightVertices.size(), "Light vertices and path do not match");

			handleCameraPath(session, cameraPath, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(cameraPath.currentSize() == mCameraVertices.size(), "Camera vertices and path do not match");

			// Handle connections (we skip 0 and camera t==1 as it is handled inside path creation)
			for (size_t t = 2; t < mCameraVertices.size(); ++t) {
				for (size_t s = 1; s < mLightVertices.size(); ++s) {
					handleConnection(session, fullPath, t, cameraPath, s, lightPath, light);
				}
			}

			// Reset
			cameraPath.popTokenUntil(1); // Keep first token
			mCameraVertices.clear();

			lightPath.popTokenUntil(0); // Do NOT keep first token!
			mLightVertices.clear();
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
	const BiDiParameters mParameters;
	const std::shared_ptr<LightSampler> mLightSampler;

	LightPathWalker mLightPathWalker;
	CameraPathWalker mCameraPathWalker;

	std::vector<BiDiPathVertex> mLightVertices;
	std::vector<BiDiPathVertex> mCameraVertices;
}; // namespace PR

template <BiDiMISMode MISMode>
class IntBiDi : public IIntegrator {
public:
	explicit IntBiDi(const BiDiParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntBiDi() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntBiDiInstance<MISMode>>(ctx, mParameters);
	}

private:
	const BiDiParameters mParameters;
};

class IntBiDiFactory : public IIntegratorFactory {
public:
	explicit IntBiDiFactory(const ParameterGroup& params)
	{
		mParameters.MaxCameraRayDepthHard = (size_t)params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = (size_t)params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard);
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));

		std::string mode = params.getString("mis", "balance");
		std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
		if (mode == "power")
			mMISMode = BM_Power;
		else
			mMISMode = BM_Balance;
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mMISMode) {
		default:
		case BM_Balance:
			return std::make_shared<IntBiDi<BM_Balance>>(mParameters);
		case BM_Power:
			return std::make_shared<IntBiDi<BM_Power>>(mParameters);
		}
	}

private:
	BiDiParameters mParameters;
	BiDiMISMode mMISMode;
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