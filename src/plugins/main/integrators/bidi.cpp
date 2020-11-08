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
// TODO: Add russian roulette pdf to MIS

namespace PR {
constexpr float DISTANCE_EPS = 0.00001f;
constexpr float GEOMETRY_EPS = 0.00001f;

static inline float culling(float cos)
{
#ifdef PR_NO_CULLING
	return std::abs(cos);
#else
	return std::max(0.0f, cos);
#endif
}

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

	inline float toPDFArea(float pdfS, const BiDiPathVertex& to) const
	{
		const Vector3f d  = to.IP.P - IP.P;
		const float dist2 = d.squaredNorm();
		if (dist2 <= PR_EPSILON)
			return 0.0f;
		if (to.IP.isAtSurface())
			pdfS *= std::abs(to.IP.Surface.N.dot(d.normalized()));
		return pdfS / dist2;
	}

	inline float pdfA(const RenderTileSession& session, const BiDiPathVertex& prev, const BiDiPathVertex& next) const
	{
		if (!Material)
			return 0.0f;

		Vector3f d_prev		   = prev.IP.P - IP.P;
		const float dist2_prev = d_prev.squaredNorm();
		if (dist2_prev <= PR_EPSILON)
			return 0.0f;
		d_prev.normalize();

		Vector3f d_next		   = next.IP.P - IP.P;
		const float dist2_next = d_next.squaredNorm();
		if (dist2_next <= PR_EPSILON)
			return 0.0f;
		d_next.normalize();

		MaterialEvalInput min;
		min.Context		   = MaterialEvalContext::fromIP(IP, d_prev, d_next);
		min.ShadingContext = ShadingContext::fromIP(session.threadID(), IP);
		MaterialPDFOutput mout;
		Material->pdf(min, mout, session);

		float pdfS = mout.PDF_S[0]; // Only hero wavelength
		if (next.IP.isAtSurface())
			pdfS *= std::abs(next.IP.Surface.N.dot(d_next));
		return pdfS / dist2_next;
	}

	static inline BiDiPathVertex fromSurface(const IntersectionPoint& ip,
											 IEntity* entity, IMaterial* material,
											 const SpectralBlob& alpha, float forwardPDF_S,
											 const BiDiPathVertex& prev)
	{
		BiDiPathVertex vertex = BiDiPathVertex{ ip, entity, material,
												alpha, 0.0f, 0.0f,
												material ? material->hasDeltaDistribution() : false };
		vertex.ForwardPDF_A	  = prev.toPDFArea(forwardPDF_S, vertex);
		return vertex;
	}
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

		mLightVertices.reserve(mParameters.MaxLightRayDepthHard + 1);
		mCameraVertices.reserve(mParameters.MaxCameraRayDepthHard + 1);
	}

	virtual ~IntBiDiInstance() = default;

	/// Apply the MIS function to the given term
	inline static float mis_term(float a)
	{
		if (a == 0.0f)
			a = 1.0f;

		if constexpr (MISMode == BM_Power)
			return a * a;
		else
			return a;
	}

	struct WalkContext {
		SpectralBlob Alpha;
		float ForwardPDF_S;
		float BackwardPDF_S;
	};

	template <bool IsCamera>
	std::optional<Ray> handleVertex(RenderTileSession& session, LightPath& path, const IntersectionPoint& ip,
									IEntity* entity, IMaterial* material, WalkContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd = session.tile()->random();

		session.tile()->statistics().addEntityHitCount();
		if constexpr (IsCamera)
			session.tile()->statistics().addCameraDepthCount();
		else
			session.tile()->statistics().addLightDepthCount();

		const auto sc = ShadingContext::fromIP(session.threadID(), ip);

		std::vector<BiDiPathVertex>& vertices = IsCamera ? mCameraVertices : mLightVertices;

		vertices.emplace_back(BiDiPathVertex::fromSurface(ip, entity, material, current.Alpha, current.ForwardPDF_S, vertices.back()));

		if constexpr (IsCamera) {
			// Handle C_0t (s==0)
			if (entity->hasEmission()
				&& culling(-ip.Surface.NdotV) > PR_EPSILON /* Only lights facing forward */) {
				IEmission* ems = session.getEmission(ip.Surface.Geometry.EmissionID);
				if (PR_LIKELY(ems)) {
					// Evaluate light
					EmissionEvalInput inL;
					inL.Entity		   = entity;
					inL.ShadingContext = sc;
					EmissionEvalOutput outL;
					ems->eval(inL, outL, session);

					const float mis = 1 / (1 + cameraMISDenomLightHit(session, vertices.size()));

					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob(mis), current.Alpha, outL.Radiance, ip.Ray, path);
					path.popToken();
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
				path.addToken(MST_DiffuseReflection);
				return {};
			}
		}

		// TODO: Add volume support
		if (!material) {
			path.addToken(MST_DiffuseReflection);
			return {};
		}

		// Sample Material
		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = sc;
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		path.addToken(sout.Type);

		const Vector3f L = sout.globalL(ip);

		if (!material->hasDeltaDistribution()) {
			sout.Weight /= sout.PDF_S[0];
			current.ForwardPDF_S = sout.PDF_S[0];

			// Calculate backward/reverse pdf
			MaterialEvalInput ein;
			ein.Context		   = MaterialEvalContext::fromIP(ip, L, -ip.Ray.Direction); // Reverse
			ein.ShadingContext = sc;
			MaterialPDFOutput pout;
			material->pdf(ein, pout, session);
			current.BackwardPDF_S = pout.PDF_S[0];
		} else {
			current.ForwardPDF_S  = 1;
			current.BackwardPDF_S = 1;
		}

		/*current.ForwardPDF_S *= scatProb;
		current.BackwardPDF_S *= scatProb; // Really?*/
		current.Alpha *= sout.Weight / scatProb;

		// Update previous backward pdf
		vertices[vertices.size() - 2].BackwardPDF_A = vertices.back().toPDFArea(current.BackwardPDF_S, vertices[vertices.size() - 2]);

		if constexpr (!IsCamera)
			current.Alpha *= IntegratorUtils::correctShadingNormalForLight(-ip.Ray.Direction, L, ip.Surface.N, ip.Surface.Geometry.N);

		if (material->isSpectralVarying())
			current.Alpha *= SpectralBlobUtils::HeroOnly();

		if (current.Alpha.isZero(PR_EPSILON) || PR_UNLIKELY(current.ForwardPDF_S <= PR_EPSILON))
			return {};

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

		session.pushSPFragment(spt, path);

		// Early drop out for invalid splashes
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

		// Initial camera vertex
		mCameraVertices.emplace_back(BiDiPathVertex{ IntersectionPoint::forPoint(spt.Ray.Origin), nullptr, nullptr, SpectralBlob::Ones(), 1, 0, true });
		WalkContext current = WalkContext{ SpectralBlob::Ones(), 1.0f, 0.0f };

		mCameraPathWalker.traverse(
			session, spt, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleVertex<true>(session, path, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				IntegratorUtils::handleBackground(
					session, ray,
					[&](const InfiniteLightEvalOutput& ileout) {
						const float mis = 1 / (1 + cameraMISDenomInfHit(session, mCameraVertices.size(), ray));
						path.addToken(LightPathToken::Background());
						session.pushSpectralFragment(SpectralBlob(mis), current.Alpha, ileout.Radiance, ray, path);
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
		const auto lsample = mLightSampler->sample(lsin, lsout, session);
		if (PR_UNLIKELY(!lsample.first))
			return nullptr;

		PR_ASSERT(lsout.CosLight >= 0.0f, "Expected light emission only from the front side!");

		Ray ray	   = Ray(lsout.LightPosition, -lsout.Outgoing);
		ray.Origin = Transform::safePosition(ray.Origin, ray.Direction); // Make sure no self intersection happens
		ray.Flags |= RF_Light;
		ray.WavelengthNM = lsin.WavelengthNM;

		LightPDF pdf = lsout.Position_PDF;
		pdf.Value *= lsout.Direction_PDF_S;
		pdf.Value = std::isinf(pdf.Value) ? 1 : pdf.Value;

		const SpectralBlob radiance = lsout.Radiance / (pdf.Value * lsample.second);

		if (lsample.first->isInfinite())
			path.addToken(LightPathToken::Background());
		else
			path.addToken(LightPathToken::Emissive());

		// Initial light vertex
		const float posPDF = std::isinf(lsout.Position_PDF.Value) ? 1 : lsout.Position_PDF.Value;
		mLightVertices.emplace_back(BiDiPathVertex{ IntersectionPoint::forPoint(lsout.LightPosition), nullptr, nullptr, radiance,
													posPDF * lsample.second, 0, !lsout.Position_PDF.IsArea });
		WalkContext current = WalkContext{ radiance, lsout.Direction_PDF_S, 0 };

		mLightPathWalker.traverse(
			session, ray,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleVertex<false>(session, path, ip,
										   entity2, material2, current);
			},
			[&](const Ray&) {
				// Do nothing! (as we do not support C_s0 connections)
			});

		return lsample.first;
	}

	///////////////////////////////////////
	//////// MIS
	inline float cameraMISDenom(const RenderTileSession& session, size_t t, size_t s) const
	{
		PR_ASSERT(t > 1 && s > 0, "Expected valid connection numbers"); /* This is called from NEE (s==1) aswell */

		const BiDiPathVertex& cv  = mCameraVertices[t - 1];
		const BiDiPathVertex& pcv = mCameraVertices[t - 2];
		const BiDiPathVertex& lv  = mLightVertices[s - 1];
		const BiDiPathVertex* plv = s > 1 ? &mLightVertices[s - 2] : nullptr;

		float misDenom = 0.0f;
		float pdfMul   = 1.0f;

		const float backwardP0 = plv ? lv.pdfA(session, *plv, cv) : 1.0f;
		pdfMul *= mis_term(backwardP0) / mis_term(cv.ForwardPDF_A);
		if (!pcv.IsDelta)
			misDenom += pdfMul;

		if (t >= 3) {
			const float backwardP1 = cv.pdfA(session, lv, pcv);
			pdfMul *= mis_term(backwardP1) / mis_term(pcv.ForwardPDF_A);
			if (!pcv.IsDelta && !mCameraVertices[t - 3].IsDelta)
				misDenom += pdfMul;
		}

		// Along camera path
		for (int i = t - 3; i > 0; --i) {
			pdfMul *= mis_term(mCameraVertices[i].BackwardPDF_A) / mis_term(mCameraVertices[i].ForwardPDF_A);
			if (!mCameraVertices[i].IsDelta && !mCameraVertices[i - 1].IsDelta)
				misDenom += pdfMul;
		}

		return misDenom;
	}

	inline float lightMISDenom(const RenderTileSession& session, size_t t, size_t s) const
	{
		PR_ASSERT(t > 1 && s > 1, "Expected valid connection numbers");

		const BiDiPathVertex& cv  = mCameraVertices[t - 1];
		const BiDiPathVertex& pcv = mCameraVertices[t - 2];
		const BiDiPathVertex& lv  = mLightVertices[s - 1];
		const BiDiPathVertex& plv = mLightVertices[s - 2];

		float misDenom = 0.0f;
		float pdfMul   = 1.0f;

		const float backwardP0 = cv.pdfA(session, pcv, lv);
		pdfMul *= mis_term(backwardP0) / mis_term(lv.ForwardPDF_A);
		if (!plv.IsDelta)
			misDenom += pdfMul;

		if (s >= 3) {
			const float backwardP1 = lv.pdfA(session, cv, plv);
			pdfMul *= mis_term(backwardP1) / mis_term(plv.ForwardPDF_A);
			if (!plv.IsDelta && !mLightVertices[t - 3].IsDelta)
				misDenom += pdfMul;
		}

		// Along light paths
		for (int i = s - 3; i >= 0; --i) {
			pdfMul *= mis_term(mLightVertices[i].BackwardPDF_A) / mis_term(mLightVertices[i].ForwardPDF_A);
			if (!mLightVertices[i].IsDelta && (i > 0 ? !mLightVertices[i - 1].IsDelta : true))
				misDenom += pdfMul;
		}

		return misDenom;
	}

	////// Special cases
	// s == 1
	inline float lightMISDenomNEE(const RenderTileSession& session, size_t t, const BiDiPathVertex& sampled) const
	{
		PR_ASSERT(t > 1, "Expected valid connection numbers");

		const BiDiPathVertex& cv  = mCameraVertices[t - 1];
		const BiDiPathVertex& pcv = mCameraVertices[t - 2];

		const float backwardP0 = cv.pdfA(session, pcv, sampled);
		return mis_term(backwardP0) / mis_term(sampled.ForwardPDF_A);
	}

	// s == 0 -> Surface Emitter Hit
	inline float cameraMISDenomLightHit(const RenderTileSession& session, size_t t) const
	{
		PR_UNUSED(session);
		if (t < 2)
			return 1.0f;

		const BiDiPathVertex& cv  = mCameraVertices[t - 1];
		const BiDiPathVertex& pcv = mCameraVertices[t - 2];

		float misDenom = 0.0f;
		float pdfMul   = 1.0f;

		if (cv.IP.Depth2 <= DISTANCE_EPS)
			return 0.0f;

		PR_ASSERT(cv.Entity, "Expected entity when something was hit!");

		const EntitySamplingInfo samplingInfo = { pcv.IP.P, pcv.IP.Surface.N };

		const float selectionProb = mLightSampler->pdfSelection(cv.Entity);
		auto pdf				  = mLightSampler->pdfPosition(cv.Entity, &samplingInfo);
		if (!pdf.IsArea)
			pdf.Value = IS::toArea(pdf.Value, cv.IP.Depth2, std::abs(cv.IP.Ray.Direction.dot(cv.IP.Surface.N)));

		const float backwardP0 = selectionProb * pdf.Value;
		pdfMul *= mis_term(backwardP0) / mis_term(cv.ForwardPDF_A);
		if (!pcv.IsDelta)
			misDenom += pdfMul;

		if (t >= 3) {
			const float pdfS	   = mLightSampler->pdfDirection(cv.IP.Ray.Direction, cv.Entity, &samplingInfo);
			const float backwardP1 = pcv.IP.isAtSurface() ? IS::toArea(pdfS, cv.IP.Depth2, std::abs(cv.IP.Ray.Direction.dot(pcv.IP.Surface.N))) : 0.0f;
			pdfMul *= mis_term(backwardP1) / mis_term(pcv.ForwardPDF_A);
			if (!pcv.IsDelta && !mCameraVertices[t - 3].IsDelta)
				misDenom += pdfMul;
		}

		// Along camera path
		for (int i = t - 3; i > 0; --i) {
			pdfMul *= mis_term(mCameraVertices[i].BackwardPDF_A) / mis_term(mCameraVertices[i].ForwardPDF_A);
			if (!mCameraVertices[i].IsDelta && !mCameraVertices[i - 1].IsDelta)
				misDenom += pdfMul;
		}

		return misDenom;
	}

	// s == 0 -> Infinite Light
	inline float cameraMISDenomInfHit(const RenderTileSession& session, size_t t, const Ray& ray) const
	{
		PR_UNUSED(session);
		if (t < 2)
			return 1.0f;

		const BiDiPathVertex& pcv = mCameraVertices[t - 1]; // Special case as no actual hit was recorded

		float misDenom = 0.0f;
		float pdfMul   = 1.0f;

		const float selectionProb = mLightSampler->pdfSelection(nullptr);
		const float backwardP0	  = selectionProb * mLightSampler->pdfDirection(ray.Direction, nullptr); // TODO: Solid Angle to Area?
		pdfMul *= mis_term(backwardP0) / mis_term(pcv.ForwardPDF_A);
		misDenom += pdfMul;

		// Along camera path
		for (int i = t - 2; i > 0; --i) {
			pdfMul *= mis_term(mCameraVertices[i].BackwardPDF_A) / mis_term(mCameraVertices[i].ForwardPDF_A);
			if (!mCameraVertices[i].IsDelta && !mCameraVertices[i - 1].IsDelta)
				misDenom += pdfMul;
		}

		return misDenom;
	}

	void handleNEE(RenderTileSession& session, LightPath& fullPath,
				   size_t t, const LightPath& cameraPath,
				   const Light* light)
	{
		PR_ASSERT(t > 1, "Expected valid connection numbers");

		const auto& cv						= mCameraVertices[t - 1];
		const auto& pcv						= mCameraVertices[t - 2];
		const EntitySamplingInfo sampleInfo = { cv.IP.P, cv.IP.Surface.N };

		LightSampleInput lsin;
		lsin.RND			= session.tile()->random().get4D();
		lsin.WavelengthNM	= cv.IP.Ray.WavelengthNM;
		lsin.Point			= &cv.IP;
		lsin.SamplingInfo	= &sampleInfo;
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		light->sample(lsin, lsout, session);

		// Sample light
		Vector3f L		 = (lsout.LightPosition - cv.IP.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosC	  = L.dot(cv.IP.Surface.N);
		const float cosL	  = culling(lsout.CosLight);
		const float Geometry  = std::abs(/*cosC */ cosL) / sqrD; // cosC already included in material
		const bool isFeasible = cv.Material && Geometry > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		float pdfA = lsout.Position_PDF.Value;
		if (!lsout.Position_PDF.IsArea)
			pdfA = IS::toArea(pdfA, sqrD, cosL);

		L = lsout.Outgoing;

		const SpectralBlob lightRad	 = lsout.Radiance / pdfA;
		const BiDiPathVertex sampled = { IntersectionPoint::forPoint(lsout.LightPosition), nullptr, nullptr, lightRad, lsout.Direction_PDF_S, 0.0f, false };

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Vector3f oN	 = cosC < 0 ? -cv.IP.Surface.N : cv.IP.Surface.N; // Offset normal used for safe positioning
		const Ray shadow	 = cv.IP.Ray.next(cv.IP.P, L, oN, RF_Shadow, SHADOW_RAY_MIN, distance);

		const bool isVisible	  = isFeasible && !session.traceShadowRay(shadow, distance, light->entityID());
		const SpectralBlob lightW = isVisible ? lightRad : SpectralBlob::Zero();

		// Evaluate surface
		SpectralBlob cameraW;
		MaterialScatteringType matST;
		if (isVisible) {
			MaterialEvalInput in{ MaterialEvalContext::fromIP(cv.IP, L), ShadingContext::fromIP(session.threadID(), cv.IP) };
			MaterialEvalOutput out;
			cv.Material->eval(in, out, session);
			cameraW = out.Weight;
			matST	= out.Type;
		} else {
			cameraW = SpectralBlob::Zero();
			matST	= MST_DiffuseReflection;
		}

		// Extract terms
		const SpectralBlob weight  = pcv.Alpha;
		const SpectralBlob contrib = lightW * Geometry * cameraW;

		const float mis = 1 / (1 + cameraMISDenom(session, t, 1) + lightMISDenomNEE(session, t, sampled));

		// Construct LPE path
		fullPath.reset();
		for (size_t t2 = 0; t2 < t - 1; ++t2)
			fullPath.addToken(cameraPath.token(t2));

		fullPath.addToken(matST);

		if (light->isInfinite())
			fullPath.addToken(LightPathToken::Background());
		else
			fullPath.addToken(LightPathToken::Emissive());

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), weight, contrib, cv.IP.Ray, fullPath);
	}

	void handleBiDiConnection(RenderTileSession& session, LightPath& fullPath,
							  size_t t, const LightPath& cameraPath,
							  size_t s, const LightPath& lightPath)
	{
		PR_ASSERT(t > 1 && s > 1, "Expected valid connection numbers");

		const auto& cv	= mCameraVertices[t - 1];
		const auto& lv	= mLightVertices[s - 1];
		const auto& pcv = mCameraVertices[t - 2];
		const auto& plv = mLightVertices[s - 2];

		Vector3f cD		  = (lv.IP.P - cv.IP.P); // Camera Vertex -> Light Vertex
		const float dist2 = cD.squaredNorm();
		const float dist  = std::sqrt(dist2);
		cD.normalize();

		const float cosC	  = cD.dot(cv.IP.Surface.N);
		const float cosL	  = culling(-cD.dot(lv.IP.Surface.N));
		const float Geometry  = std::abs(/*cosC */ cosL) / dist2;  // cosC already included in material
		const bool isFeasible = lv.Material && cv.Material
								&& Geometry > GEOMETRY_EPS && dist2 > DISTANCE_EPS;

		const Vector3f oN = cosC < 0 ? -cv.IP.Surface.N : cv.IP.Surface.N; // Offset normal used for safe positioning
		const Ray shadow  = cv.IP.Ray.next(cv.IP.P, cD, oN, RF_Shadow, SHADOW_RAY_MIN, dist);

		// Extract visible and geometry term
		const bool isVisible = isFeasible && !session.traceShadowRay(shadow, dist, lv.Entity->id());

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
		const SpectralBlob weight  = plv.Alpha * pcv.Alpha;
		const SpectralBlob contrib = lightW * Geometry * cameraW;

		const float mis = 1 / (1 + cameraMISDenom(session, t, s) + lightMISDenom(session, t, s));

		// Construct LPE path
		fullPath.reset();
		for (size_t t2 = 0; t2 < t; ++t2)
			fullPath.addToken(cameraPath.token(t2));
		for (size_t s2 = 0; s2 < s; ++s2)
			fullPath.addToken(lightPath.token(s - 1 - s2));

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), weight, contrib, cv.IP.Ray, fullPath);
	}

	void handleConnection(RenderTileSession& session, LightPath& fullPath,
						  size_t t, const LightPath& cameraPath,
						  size_t s, const LightPath& lightPath, const Light* light)
	{
		PR_ASSERT(t > 1 && s > 0, "Expected valid connection numbers");

		if (s == 1)
			handleNEE(session, fullPath, t, cameraPath, light);
		else
			handleBiDiConnection(session, fullPath, t, cameraPath, s, lightPath);
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
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
			PR_ASSERT(lightPath.currentSize() == mLightVertices.size(), "Light vertices and path do not match");

			handleCameraPath(session, cameraPath, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(cameraPath.currentSize() == mCameraVertices.size(), "Camera vertices and path do not match");

			// Handle connections (we skip 0 and camera t==1 as it is handled inside path creation)
			for (size_t t = 2; t <= mCameraVertices.size(); ++t)
				for (size_t s = 1; s <= mLightVertices.size(); ++s)
					handleConnection(session, fullPath, t, cameraPath, s, lightPath, light);

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