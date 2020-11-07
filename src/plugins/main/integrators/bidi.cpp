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
	bool IsDelta;
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

	struct WalkContext {
		float ForwardPDF_S;
		float BackwardPDF_S;
	};

	template <bool IsCamera>
	std::optional<Ray> handleVertex(RenderTileSession& session, LightPath& path, const IntersectionPoint& ip,
									IEntity* entity, IMaterial* material, WalkContext& current)
	{
		if (!entity || !material) // Nothing found, abort
			return {};

		auto& rnd = session.tile()->random();

		session.tile()->statistics().addEntityHitCount();
		if constexpr (IsCamera)
			session.tile()->statistics().addCameraDepthCount();
		else
			session.tile()->statistics().addLightDepthCount();

		const auto sc = ShadingContext::fromIP(session.threadID(), ip);

		std::vector<BiDiPathVertex>& vertices = IsCamera ? mCameraVertices : mLightVertices;
		const SpectralBlob prevAlpha		  = vertices[ip.Ray.IterationDepth].Alpha;

		// Add pdf which generated this vertex
		float forwardPDF_A	= current.ForwardPDF_S;
		float backwardPDF_A = current.BackwardPDF_S;
		if (!vertices[ip.Ray.IterationDepth].IsDelta) {
			forwardPDF_A *= IS::toArea(forwardPDF_A, ip.Depth2, std::abs(ip.Surface.NdotV));
			backwardPDF_A *= IS::toArea(backwardPDF_A, ip.Depth2, std::abs(ip.Surface.NdotV) /* Not ndotl? */);
		}

		if constexpr (IsCamera) {
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
						path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
						session.pushSpectralFragment(SpectralBlob::Ones(), prevAlpha, outL.Radiance, ip.Ray, path);
						path.popToken();
					}
				}
			}
		}
		// Our implementation does not support direct camera hits (no C_s0)

		// Russian roulette
		float scatProb				 = 1.0f;
		const size_t maxRayDepthSoft = IsCamera ? mParameters.MaxCameraRayDepthSoft : mParameters.MaxLightRayDepthSoft;
		if (ip.Ray.IterationDepth >= maxRayDepthSoft) {
			constexpr float SCATTER_EPS = 1e-4f;

			const float russian_prob = rnd.getFloat();
			scatProb				 = std::min<float>(1.0f, std::pow(0.8f, ip.Ray.IterationDepth - maxRayDepthSoft));
			if (russian_prob > scatProb || scatProb <= SCATTER_EPS) {
				// Stop traversal with dark vertex
				path.addToken(MST_DiffuseReflection);
				vertices.emplace_back(BiDiPathVertex{ ip, entity, material, SpectralBlob::Zero(), forwardPDF_A, backwardPDF_A, material->hasDeltaDistribution() });
				return {};
			}
		}

		// Sample Material
		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = sc;
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		const Vector3f L = sout.globalL(ip);

		if (!material->hasDeltaDistribution()) {
			sout.Weight /= scatProb * sout.PDF_S[0];
			current.ForwardPDF_S = scatProb * sout.PDF_S[0];

			// Calculate backward/reverse pdf
			MaterialEvalInput ein;
			ein.Context		   = MaterialEvalContext::fromIP(ip, L, -ip.Ray.Direction); // Reverse
			ein.ShadingContext = sc;
			MaterialPDFOutput pout;
			material->pdf(ein, pout, session);
			current.BackwardPDF_S = scatProb * pout.PDF_S[0];
		} else {
			current.ForwardPDF_S  = 1;
			current.BackwardPDF_S = 1;
		}

		SpectralBlob alphaM = sout.Weight * prevAlpha;
		if constexpr (!IsCamera)
			alphaM *= IntegratorUtils::correctShadingNormalForLight(-ip.Ray.Direction, L, ip.Surface.N, ip.Surface.Geometry.N);

		if (material->isSpectralVarying())
			alphaM *= SpectralBlobUtils::HeroOnly();

		if (alphaM.isZero(PR_EPSILON) || PR_UNLIKELY(sout.PDF_S[0] <= PR_EPSILON)) {
			// Stop traversal with dark vertex
			path.addToken(sout.Type);
			vertices.emplace_back(BiDiPathVertex{ ip, entity, material, SpectralBlob::Zero(), forwardPDF_A, backwardPDF_A, material->hasDeltaDistribution() });
			return {};
		}

		// Add to path
		path.addToken(sout.Type);
		vertices.emplace_back(BiDiPathVertex{ ip, entity, material, alphaM, forwardPDF_A, backwardPDF_A, material->hasDeltaDistribution() });

		// Setup ray flags
		int rflags = RF_Bounce;
		if (material->isSpectralVarying())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.Ray.next(ip.P, L, ip.Surface.N,
											  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	/////////////////// Camera Path
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
		mCameraVertices.emplace_back(BiDiPathVertex{ IntersectionPoint(), nullptr, nullptr, SpectralBlob::Ones(), 1, 1, true });
		WalkContext current = WalkContext{ 1.0f, 1.0f };

		mCameraPathWalker.traverse(
			session, spt, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleVertex<true>(session, path, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				const SpectralBlob prevAlpha = (ray.IterationDepth == 0) ? SpectralBlob::Ones() : mCameraVertices[ray.IterationDepth - 1].Alpha;
				IntegratorUtils::handleBackground(
					session, ray,
					[&](const InfiniteLightEvalOutput& ileout) {
						path.addToken(LightPathToken::Background());
						session.pushSpectralFragment(SpectralBlob::Ones(), prevAlpha, ileout.Radiance, ray, path);
						path.popToken();
					});
			});
	}

	/////////////////// Light Path
	// First light vertex
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
		mLightVertices.emplace_back(BiDiPathVertex{ IntersectionPoint(), nullptr, nullptr, radiance,
													lsout.Position_PDF.Value, lsout.Position_PDF.Value, !lsout.Position_PDF.IsArea });
		WalkContext current = WalkContext{ lsout.Direction_PDF_S, lsout.Direction_PDF_S };

		mLightPathWalker.traverse(
			session, ray,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleVertex<false>(session, path, ip,
										   entity2, material2, current);
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
		float misDenom = 0.0f;

		// Along camera paths
		float pdfMul = 1.0f;
		for (int i = t - 1; i > 0; --i) {
			pdfMul *= mis_term(mCameraVertices[i].BackwardPDF_A) / mis_term(mCameraVertices[i].ForwardPDF_A);
			if (!mCameraVertices[i].IsDelta && !mCameraVertices[i - 1].IsDelta)
				misDenom += pdfMul;
		}

		// Along light paths
		pdfMul = 1.0f;
		for (int i = s - 1; i >= 0; --i) {
			pdfMul *= mis_term(mLightVertices[i].BackwardPDF_A) / mis_term(mLightVertices[i].ForwardPDF_A);
			if (!mLightVertices[i].IsDelta && (i > 0 ? !mLightVertices[i - 1].IsDelta : true))
				misDenom += pdfMul;
		}

		const float mis = 1 / (1 + misDenom);

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