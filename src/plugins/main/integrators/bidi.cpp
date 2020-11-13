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
#include "PathVertex.h"
#include "Walker.h"

#include "Logger.h"

/* Implementation of a bidirectional path tracer */
// Based on VCM without merging and direct camera connections

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

static inline float checkPDF(float a)
{
	return std::isinf(a) ? 1 : a;
}

struct BiDiParameters {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
};

enum BiDiMISMode {
	BM_Balance,
	BM_Power
};

struct CameraWalkContext {
	SpectralBlob Throughput;
	float MIS_VCM;
	float MIS_VC;
};

struct LightWalkContext : public CameraWalkContext {
	bool IsFiniteLight;
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
		, mCameraPath(mParameters.MaxCameraRayDepthHard + 2)
		, mLightPath(mParameters.MaxLightRayDepthHard + 2)
		, mFullPath(mParameters.MaxCameraRayDepthHard + mParameters.MaxLightRayDepthHard + 2)
	{
		mLightPathWalker.MaxRayDepthHard  = mParameters.MaxLightRayDepthHard;
		mLightPathWalker.MaxRayDepthSoft  = mParameters.MaxLightRayDepthSoft;
		mCameraPathWalker.MaxRayDepthHard = mParameters.MaxCameraRayDepthHard;
		mCameraPathWalker.MaxRayDepthSoft = mParameters.MaxCameraRayDepthSoft;

		mCameraPath.addToken(LightPathToken::Camera());

		mLightVertices.reserve(mParameters.MaxLightRayDepthHard + 1);
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

	template <bool IsCamera>
	inline float russianRouletteProbability(uint32 path_length)
	{
		if (path_length == 0)
			return 1.0f;

		const size_t maxRayDepthSoft = IsCamera ? mParameters.MaxCameraRayDepthSoft : mParameters.MaxLightRayDepthSoft;
		if (path_length >= maxRayDepthSoft) {
			constexpr float SCATTER_EPS = 1e-4f;
			const float scatProb		= std::min<float>(1.0f, std::pow(0.8f, path_length - maxRayDepthSoft));
			return scatProb <= SCATTER_EPS ? 0.0f : scatProb;
		} else {
			return 1.0f;
		}
	}

	template <bool IsCamera>
	inline std::optional<float> checkRoulette(RenderTileSession& session, uint32 path_length)
	{
		const float scatProb = russianRouletteProbability<IsCamera>(path_length);
		if (scatProb == 0.0f) {
			return {};
		} else if (scatProb < 1.0f) {
			const float russian_prob = session.random().getFloat();
			if (russian_prob > scatProb)
				return {};
		}

		return { scatProb };
	}

	template <bool IsCamera>
	std::optional<Ray> handleScattering(RenderTileSession& session, const IntersectionPoint& ip,
										IEntity* entity, IMaterial* material, CameraWalkContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd		= session.random();
		LightPath& path = IsCamera ? mCameraPath : mLightPath;

		// Russian roulette
		const auto roulette = checkRoulette<IsCamera>(session, ip.Ray.IterationDepth + 1);
		if (!roulette.has_value()) {
			path.addToken(MST_DiffuseReflection);
			return {};
		}
		const float scatProb = roulette.value();

		// TODO: Add volume support
		if (!material) {
			path.addToken(MST_DiffuseReflection);
			return {};
		}

		// Sample Material
		MaterialSampleInput sin;
		sin.Context		   = MaterialSampleContext::fromIP(ip);
		sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, session);

		path.addToken(sout.Type);

		const Vector3f L   = sout.globalL(ip);
		const bool isDelta = material->hasDeltaDistribution();

		if (sout.Weight.isZero())
			return {};

		// Calculate forward and backward PDFs
		float forwardPDF_S	= sout.PDF_S[0]; // Hero wavelength
		float backwardPDF_S = forwardPDF_S;	 // Same if delta distribution
		if (!isDelta) {
			// Calculate backward/reverse pdf
			MaterialEvalInput ein;
			ein.Context		   = MaterialEvalContext::fromIP(ip, L, -ip.Ray.Direction); // Reverse
			ein.ShadingContext = sin.ShadingContext;
			MaterialPDFOutput pout;
			material->pdf(ein, pout, session);
			backwardPDF_S = pout.PDF_S[0];
		}

		forwardPDF_S *= scatProb;
		backwardPDF_S *= scatProb;

		// Second time update MIS terms
		const float NdotL = std::abs(sout.L[2]);
		if (isDelta) {
			current.MIS_VC *= mis_term(NdotL); // NdotL
			current.MIS_VCM = 0;
		} else {
			current.MIS_VC	= mis_term(NdotL / backwardPDF_S) * (current.MIS_VC * mis_term(backwardPDF_S) + current.MIS_VCM);
			current.MIS_VCM = mis_term(1 / backwardPDF_S);
		}

		// Update throughput
		current.Throughput *= sout.Weight / backwardPDF_S;
		if constexpr (IsCamera)
			current.Throughput *= IntegratorUtils::correctShadingNormalForLight(-ip.Ray.Direction, L, ip.Surface.N, ip.Surface.Geometry.N);

		if (material->isSpectralVarying())
			current.Throughput *= SpectralBlobUtils::HeroOnly();

		if (current.Throughput.isZero(PR_EPSILON))
			return {};

		// Setup ray flags
		int rflags = RF_Bounce;
		if (material->isSpectralVarying())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.Ray.next(ip.P, L, ip.Surface.N,
											  rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	std::optional<Ray> handleLightVertex(RenderTileSession& session, const IntersectionPoint& ip,
										 IEntity* entity, IMaterial* material, LightWalkContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addLightDepthCount();

		const uint32 pathLength = ip.Ray.IterationDepth + 1;

		// Update the MIS quantities before storing them at the vertex.
		if (pathLength > 1 || current.IsFiniteLight)
			current.MIS_VCM *= mis_term(ip.Depth2);

		if (ip.isAtSurface()) {
			current.MIS_VCM /= mis_term(std::abs(ip.Surface.NdotV));
			current.MIS_VC /= mis_term(std::abs(ip.Surface.NdotV));
		}

		// If there is no material to scatter from, give up
		if (!material)
			return {};

		// Store vertex, unless material is purely specular
		if (!material->hasDeltaDistribution()) {
			PathVertex vertex;
			vertex.IP		  = ip;
			vertex.Throughput = current.Throughput;
			vertex.MIS_VCM	  = current.MIS_VCM;
			vertex.MIS_VC	  = current.MIS_VC;
			vertex.MIS_VM	  = 0; // Not used
			mLightVertices.emplace_back(vertex);
		}
		// Our implementation does not support direct camera hits (no C_s0)

		return handleScattering<false>(session, ip, entity, material, current);
	}

	std::optional<Ray> handleCameraVertex(RenderTileSession& session, const IntersectionPoint& ip,
										  IEntity* entity, IMaterial* material, CameraWalkContext& current)
	{
		PR_ASSERT(entity, "Expected valid entity");

		session.tile()->statistics().addEntityHitCount();
		session.tile()->statistics().addCameraDepthCount();

		// Update the MIS quantities before computing the vertex.
		current.MIS_VCM *= mis_term(ip.Depth2);
		if (ip.isAtSurface()) {
			current.MIS_VCM /= mis_term(std::abs(ip.Surface.NdotV));
			current.MIS_VC /= mis_term(std::abs(ip.Surface.NdotV));
		}

		// Handle light emission (c0t)
		if (entity->hasEmission()) {
			mCameraPath.addToken(LightPathToken::Emissive());
			handleDirectHit(session, ip, entity, current);
			mCameraPath.popToken();
		}

		// If there is no material to scatter from, give up
		if (!material)
			return {};

		if (!material->hasDeltaDistribution()) {
			// c1t
			handleNEE(session, ip, material, current);
			// cst
			for (const auto& vertex : mLightVertices)
				handleConnection(session, vertex, ip, material, current);
		}

		return handleScattering<true>(session, ip, entity, material, current);
	}

	/////////////////// Camera Path
	// First camera vertex
	void handleCameraPath(RenderTileSession& session, const IntersectionPoint& spt,
						  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		session.pushSPFragment(spt, mCameraPath);

		// Early drop out for invalid splashes
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

		// Initial camera vertex
		CameraWalkContext current = CameraWalkContext{ SpectralBlob::Ones(), 0.0f, 0.0f };

		mCameraPathWalker.traverse(
			session, spt, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(session, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				mCameraPath.addToken(LightPathToken::Background());
				handleInfLights(session, current, ray);
				mCameraPath.popToken();
			});
	}

	/////////////////// Light Path
	// First light vertex
	const Light* handleLightPath(RenderTileSession& session, const SpectralBlob& wvl)
	{
		PR_PROFILE_THIS;

		// Sample light
		LightSampleInput lsin;
		lsin.WavelengthNM	= wvl;
		lsin.RND			= session.random().get4D();
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, session);
		if (PR_UNLIKELY(!lsample.first))
			return nullptr;

		PR_ASSERT(lsout.CosLight >= 0.0f, "Expected light emission only from the front side!");
		const Light* light = lsample.first;

		// Calculate PDF
		const float directionPdf_S = lsout.Direction_PDF_S * lsample.second;
		const float positionPdf_A  = lsout.Position_PDF.Value * lsample.second; // TODO: What if not area?

		// Setup light path context
		LightWalkContext current = LightWalkContext{ lsout.Radiance, 0, 0, !light->isInfinite() };

		current.Throughput /= directionPdf_S;
		current.MIS_VCM = mis_term(positionPdf_A / directionPdf_S);
		if (light->hasDeltaDistribution())
			current.MIS_VC = 0;
		else
			current.MIS_VC = mis_term(lsout.CosLight / directionPdf_S);

		if (light->isInfinite())
			mLightPath.addToken(LightPathToken::Background());
		else
			mLightPath.addToken(LightPathToken::Emissive());

		// Construct direction
		Ray ray	   = Ray(lsout.LightPosition, -lsout.Outgoing);
		ray.Origin = Transform::safePosition(ray.Origin, ray.Direction); // Make sure no self intersection happens
		ray.Flags |= RF_Light;
		ray.WavelengthNM = lsin.WavelengthNM;

		mLightPathWalker.traverse(
			session, ray,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleLightVertex(session, ip,
										 entity2, material2, current);
			},
			[&](const Ray&) {
				// Do nothing! (as we do not support C_s0 connections)
			});

		return light;
	}

	inline SpectralBlob extractMaterial(const RenderTileSession& session,
										const IntersectionPoint& ip, const IMaterial* material, const Vector3f& d,
										float& forwardPDF_S, float& backwardPDF_S, MaterialScatteringType& type)
	{
		// Forward
		MaterialEvalInput in;
		in.Context		  = MaterialEvalContext::fromIP(ip, d);
		in.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);
		MaterialEvalOutput out;
		material->eval(in, out, session);

		// Backward/reverse pdf
		in.Context = MaterialEvalContext::fromIP(ip, d, -ip.Ray.Direction); // Reverse
		MaterialPDFOutput pout;
		material->pdf(in, pout, session);

		forwardPDF_S  = out.PDF_S[0];
		backwardPDF_S = pout.PDF_S[0];
		type		  = out.Type;
		return out.Weight;
	}

	void handleNEE(RenderTileSession& session,
				   const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, const CameraWalkContext& current)
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

		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const float cameraRoulette	  = russianRouletteProbability<true>(cameraPathLength);

		// Calculate geometry stuff
		const float sqrD	  = (lsout.LightPosition - cameraIP.P).squaredNorm();
		const Vector3f L	  = lsout.Outgoing;
		const float cosC	  = L.dot(cameraIP.Surface.N);
		const float cosL	  = culling(lsout.CosLight);
		const float Geometry  = std::abs(cosC * cosL) / sqrD;
		const bool isFeasible = Geometry > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		if (!isFeasible)
			return; // TODO: Add weights?

		// Evaluate camera material
		float cameraForwardPDF_S;
		float cameraBackwardPDF_S;
		MaterialScatteringType cameraScatteringType;
		const SpectralBlob cameraW = extractMaterial(session, cameraIP, cameraMaterial, L, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);

		if (!light->hasDeltaDistribution())
			cameraForwardPDF_S *= cameraRoulette;
		else
			cameraForwardPDF_S = 0;
		cameraBackwardPDF_S *= cameraRoulette;

		// Convert position pdf to solid angle if necessary
		float positionPDF_S = lsout.Position_PDF.Value;
		if (lsout.Position_PDF.IsArea)
			positionPDF_S = IS::toSolidAngle(positionPDF_S, sqrD, cosL);

		// Calculate MIS
		const float lightMIS  = mis_term(cameraForwardPDF_S / (lsample.second * positionPDF_S));
		const float cameraMIS = mis_term(lsout.Direction_PDF_S * cosC / (positionPDF_S * cosL))
								* (current.MIS_VCM + current.MIS_VC * mis_term(cameraBackwardPDF_S));

		const float mis = 1 / (1 + lightMIS + cameraMIS);

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Vector3f oN	 = cosC < 0 ? -cameraIP.Surface.N : cameraIP.Surface.N; // Offset normal used for safe positioning
		const Ray shadow	 = cameraIP.Ray.next(cameraIP.P, L, oN, RF_Shadow, SHADOW_RAY_MIN, distance);

		const bool isVisible	  = !session.traceShadowRay(shadow, distance, light->entityID());
		const SpectralBlob lightW = isVisible ? lsout.Radiance : SpectralBlob::Zero();

		// Calculate contribution
		const SpectralBlob contrib = lightW * cameraW / (lsample.second * positionPDF_S);

		// Construct LPE path
		mFullPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			mFullPath.addToken(mCameraPath.token(t2));

		mFullPath.addToken(cameraScatteringType);

		if (light->isInfinite())
			mFullPath.addToken(LightPathToken::Background());
		else
			mFullPath.addToken(LightPathToken::Emissive());

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib,
									 cameraIP.Ray, mFullPath);
	}

	void handleConnection(RenderTileSession& session,
						  const PathVertex& lightVertex,
						  const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, const CameraWalkContext& current)
	{
		Vector3f cD		  = (lightVertex.IP.P - cameraIP.P); // Camera Vertex -> Light Vertex
		const float dist2 = cD.squaredNorm();
		const float dist  = std::sqrt(dist2);
		cD /= dist;

		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const uint32 lightPathLength  = lightVertex.pathLength();

		// Extract necessary materials
		const IMaterial* lightMaterial = session.getMaterial(lightVertex.IP.Surface.Geometry.MaterialID);
		PR_ASSERT(lightMaterial, "Expected valid material for light vertex");

		const float cameraRoulette = russianRouletteProbability<true>(cameraPathLength);
		const float lightRoulette  = russianRouletteProbability<false>(lightPathLength);

		// Evaluate camera material
		float cameraForwardPDF_S;
		float cameraBackwardPDF_S;
		MaterialScatteringType cameraScatteringType;
		const SpectralBlob cameraW = extractMaterial(session, cameraIP, cameraMaterial, cD, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);
		cameraForwardPDF_S *= cameraRoulette;
		cameraBackwardPDF_S *= cameraRoulette;

		// Evaluate light material
		float lightForwardPDF_S;
		float lightBackwardPDF_S;
		MaterialScatteringType lightScatteringType; // Unused
		const SpectralBlob lightW = extractMaterial(session, lightVertex.IP, lightMaterial, -cD, lightForwardPDF_S, lightBackwardPDF_S, lightScatteringType);
		lightForwardPDF_S *= lightRoulette;
		lightBackwardPDF_S *= lightRoulette;

		// Calculate geometry term
		const float cosC	  = cD.dot(cameraIP.Surface.N);
		const float cosL	  = culling(-cD.dot(lightVertex.IP.Surface.N));
		const float Geometry  = std::abs(/*cosC */ cosL) / dist2; // cosC already included in material
		const bool isFeasible = Geometry > GEOMETRY_EPS && dist2 > DISTANCE_EPS;
		if (!isFeasible)
			return;

		// To Area
		const float cameraForwardPDF_A = IS::toArea(cameraForwardPDF_S, dist2, cosL);
		const float lightForwardPDF_A  = IS::toArea(lightForwardPDF_S, dist2, std::abs(cosC));

		// Calculate MIS
		const float cameraMIS = mis_term(cameraForwardPDF_A) * (lightVertex.MIS_VCM + lightVertex.MIS_VC * mis_term(lightBackwardPDF_S));
		const float lightMIS  = mis_term(lightForwardPDF_A) * (current.MIS_VCM + current.MIS_VC * mis_term(cameraBackwardPDF_S));
		const float mis		  = 1 / (1 + lightMIS + cameraMIS);

		// Construct ray
		const Vector3f oN = cosC < 0 ? -cameraIP.Surface.N : cameraIP.Surface.N; // Offset normal used for safe positioning
		const Ray shadow  = cameraIP.Ray.next(cameraIP.P, cD, oN, RF_Shadow, SHADOW_RAY_MIN, dist);

		// Extract visible and geometry term
		const bool isVisible = !session.traceShadowRay(shadow, dist, lightVertex.IP.Surface.Geometry.EntityID);

		// Extract terms
		const SpectralBlob contrib = isVisible ? (lightW * Geometry * cameraW).eval() : SpectralBlob::Zero();

		// Construct LPE path
		mFullPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			mFullPath.addToken(mCameraPath.token(t2));

		mFullPath.addToken(cameraScatteringType);

		for (size_t s2 = 0; s2 < lightPathLength; ++s2)
			mFullPath.addToken(mLightPath.token(lightPathLength - 1 - s2));

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib, cameraIP.Ray, mFullPath);
	}

	// Handle case where camera ray directly hits emissive object
	void handleDirectHit(RenderTileSession& session, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, const CameraWalkContext& current)
	{
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;

		const IEmission* ems = session.getEmission(cameraIP.Surface.Geometry.EmissionID);
		if (PR_UNLIKELY(!ems))
			return;

		// Cull emission if needed
		if (culling(-cameraIP.Surface.NdotV) <= PR_EPSILON)
			return;

		// Evaluate emission
		EmissionEvalInput ein;
		ein.Entity		   = cameraEntity;
		ein.ShadingContext = ShadingContext::fromIP(session.threadID(), cameraIP);
		EmissionEvalOutput eout;
		ems->eval(ein, eout, session);

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, eout.Radiance, cameraIP.Ray, mCameraPath);
			return;
		}

		// Evaluate PDF
		const float selProb = mLightSampler->pdfSelection(cameraEntity);
		auto posPDF			= mLightSampler->pdfPosition(cameraEntity);
		PR_ASSERT(posPDF.IsArea, "Area lights should return pdfs respective to area!");
		float dirPDF_S = mLightSampler->pdfDirection(cameraIP.Ray.Direction, cameraEntity);

		const float posPDF_A = posPDF.Value * selProb;
		dirPDF_S *= selProb;

		// Calculate MIS
		const float cameraMIS = mis_term(posPDF_A) * current.MIS_VCM + mis_term(dirPDF_S) * current.MIS_VC;
		const float mis		  = 1 / (1 + cameraMIS);

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, eout.Radiance, cameraIP.Ray, mCameraPath);
	}

	// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const RenderTileSession& session, const CameraWalkContext& current, const Ray& ray)
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;

		// Evaluate radiance
		float dirPDF_S		  = 0;
		SpectralBlob radiance = SpectralBlob::Zero();
		for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->eval(lin, lout, session);

			radiance += lout.Radiance;
			dirPDF_S += lout.Direction_PDF_S;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, ray, mCameraPath);
			return;
		}

		// Evaluate PDF (for now independent of concrete inf light)
		const float selProb = mLightSampler->pdfSelection(nullptr);
		auto posPDF			= mLightSampler->pdfPosition(nullptr);

		float posPDF_A = posPDF.Value;
		if (!posPDF.IsArea) {
			const float r = session.context()->scene()->boundingSphere().radius();
			posPDF_A	  = IS::toArea(posPDF_A, r * r, 1);
		}

		posPDF_A *= selProb;
		dirPDF_S *= selProb;

		// Calculate MIS
		const float cameraMIS = mis_term(posPDF_A) * current.MIS_VCM + mis_term(dirPDF_S) * current.MIS_VC;
		const float mis		  = 1 / (1 + cameraMIS);

		// Splat
		session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, radiance, ray, mCameraPath);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			// Trace necessary paths
			const Light* light = handleLightPath(session, spt.Ray.WavelengthNM);
			if (PR_UNLIKELY(!light))
				return; // Giveup as no light is present
			PR_ASSERT(mLightPath.currentSize() > mLightVertices.size(), "Light vertices and path do not match");

			handleCameraPath(session, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));

			// Reset
			mCameraPath.popTokenUntil(1); // Keep first token

			mLightPath.popTokenUntil(0); // Do NOT keep first token!
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

	std::vector<PathVertex> mLightVertices;

	// Context of evaluation
	LightPath mCameraPath;
	LightPath mLightPath;
	LightPath mFullPath;
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