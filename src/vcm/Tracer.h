#pragma once

#include "Options.h"
#include "PathVertex.h"
#include "RussianRoulette.h"
#include "TracerContext.h"
#include "TraversalContext.h"
#include "Utils.h"
#include "Walker.h"
#include "renderer/RenderTileSession.h"

namespace PR {
namespace VCM {

template <bool UseConnection, bool UseMerging, bool HasInfLights, MISMode Mode = MM_Balance>
class Tracer {
private:
	/// Apply the MIS function to the given term
	inline static float mis_term(float a)
	{
		if constexpr (Mode == MM_Power)
			return a * a;
		else
			return a;
	}

public:
	inline Tracer(const Options& options, const std::shared_ptr<LightSampler>& lightSampler)
		: mOptions(options)
		, mCameraWalker(options.MaxCameraRayDepthHard)
		, mCameraRR(options.MaxCameraRayDepthSoft)
		, mLightWalker(options.MaxLightRayDepthHard)
		, mLightRR(options.MaxLightRayDepthSoft)
		, mLightSampler(lightSampler)
	{
	}

	inline const Options& options() const { return mOptions; }

	inline void traceCameraPath(TracerContext& tctx, const IntersectionPoint& initial_hit,
								IEntity* entity, IMaterial* material) const
	{
		PR_PROFILE_THIS;

		tctx.Session.pushSPFragment(initial_hit, tctx.CameraPath);

		// Early drop out for invalid splashes
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

		// Initial camera vertex
		CameraTraversalContext current = CameraTraversalContext{ SpectralBlob::Ones(), 0.0f, 0.0f, 0.0f };

		mCameraWalker.traverse(
			tctx.Session, initial_hit, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(tctx, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				tctx.CameraPath.addToken(LightPathToken::Background());
				handleInfLights(tctx, current, ray);
				tctx.CameraPath.popToken();
			});
	}

	// First light vertex
	const Light* traceLightPath(TracerContext& tctx, const SpectralBlob& wvl) const
	{
		PR_PROFILE_THIS;

		// Sample light
		LightSampleInput lsin;
		lsin.WavelengthNM	= wvl;
		lsin.RND			= tctx.Session.random().get4D();
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, tctx.Session);
		if (PR_UNLIKELY(!lsample.first))
			return nullptr;

		PR_ASSERT(lsout.CosLight >= 0.0f, "Expected light emission only from the front side!");
		const Light* light = lsample.first;

		// Calculate PDF
		const float directionPdf_S = lsout.Direction_PDF_S * lsample.second;
		const float positionPdf_A  = lsout.Position_PDF.Value * lsample.second; // TODO: What if not area?

		// Setup light path context
		LightTraversalContext current = LightTraversalContext{ lsout.Radiance, 0, 0, 0, !light->isInfinite() };

		current.Throughput /= directionPdf_S;
		current.MIS_VCM = mis_term(positionPdf_A / directionPdf_S);
		if (light->hasDeltaDistribution())
			current.MIS_VC = 0;
		else
			current.MIS_VC = mis_term(lsout.CosLight / directionPdf_S);

		if (light->isInfinite())
			tctx.LightPath.addToken(LightPathToken::Background());
		else
			tctx.LightPath.addToken(LightPathToken::Emissive());

		// Construct direction
		Ray ray	   = Ray(lsout.LightPosition, -lsout.Outgoing);
		ray.Origin = Transform::safePosition(ray.Origin, ray.Direction); // Make sure no self intersection happens
		ray.Flags |= RF_Light;
		ray.WavelengthNM = lsin.WavelengthNM;

		mLightWalker.traverse(
			tctx.Session, ray,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleLightVertex(tctx, ip,
										 entity2, material2, current);
			},
			[&](const Ray&) {
				// Do nothing! (as we do not support C_s0 connections)
			});

		return light;
	}

private:
	template <bool IsCamera>
	inline auto checkRoulette(Random& rnd, uint32 path_length) const
	{
		if constexpr (IsCamera)
			return mCameraRR.check(rnd, path_length);
		else
			return mLightRR.check(rnd, path_length);
	}

	template <bool IsCamera>
	std::optional<Ray> handleScattering(TracerContext& tctx, const IntersectionPoint& ip,
										IEntity* entity, IMaterial* material, CameraTraversalContext& current) const
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd		= tctx.Session.random();
		LightPath& path = IsCamera ? tctx.CameraPath : tctx.LightPath;

		// Russian roulette
		const auto roulette = checkRoulette<IsCamera>(tctx.Session.random(), ip.Ray.IterationDepth + 1);
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
		sin.ShadingContext = ShadingContext::fromIP(tctx.Session.threadID(), ip);
		sin.RND			   = rnd.get2D();

		MaterialSampleOutput sout;
		material->sample(sin, sout, tctx.Session);

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
			material->pdf(ein, pout, tctx.Session);
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
			current.Throughput *= correctShadingNormalForLight(-ip.Ray.Direction, L, ip.Surface.N, ip.Surface.Geometry.N);

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

	std::optional<Ray> handleLightVertex(TracerContext& tctx, const IntersectionPoint& ip,
										 IEntity* entity, IMaterial* material, LightTraversalContext& current) const
	{
		PR_ASSERT(entity, "Expected valid entity");

		tctx.Session.tile()->statistics().addEntityHitCount();
		tctx.Session.tile()->statistics().addLightDepthCount();

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
			tctx.LightVertices.emplace_back(vertex);
		}
		// Our implementation does not support direct camera hits (no C_s0)

		return handleScattering<false>(tctx, ip, entity, material, current);
	}

	std::optional<Ray> handleCameraVertex(TracerContext& tctx, const IntersectionPoint& ip,
										  IEntity* entity, IMaterial* material, CameraTraversalContext& current) const
	{
		PR_ASSERT(entity, "Expected valid entity");

		tctx.Session.tile()->statistics().addEntityHitCount();
		tctx.Session.tile()->statistics().addCameraDepthCount();

		// Update the MIS quantities before computing the vertex.
		current.MIS_VCM *= mis_term(ip.Depth2);
		if (ip.isAtSurface()) {
			current.MIS_VCM /= mis_term(std::abs(ip.Surface.NdotV));
			current.MIS_VC /= mis_term(std::abs(ip.Surface.NdotV));
		}

		// Handle light emission (c0t)
		if (entity->hasEmission()) {
			tctx.CameraPath.addToken(LightPathToken::Emissive());
			handleDirectHit(tctx, ip, entity, current);
			tctx.CameraPath.popToken();
		}

		// If there is no material to scatter from, give up
		if (!material)
			return {};

		if (!material->hasDeltaDistribution()) {
			// c1t
			handleNEE(tctx, ip, material, current);
			if constexpr (UseConnection) {
				// cst
				for (const auto& vertex : tctx.LightVertices)
					handleConnection(tctx, vertex, ip, material, current);
			}
		}

		return handleScattering<true>(tctx, ip, entity, material, current);
	}

	inline SpectralBlob extractMaterial(const TracerContext& tctx,
										const IntersectionPoint& ip, const IMaterial* material, const Vector3f& d,
										float& forwardPDF_S, float& backwardPDF_S, MaterialScatteringType& type) const
	{
		// Forward
		MaterialEvalInput in;
		in.Context		  = MaterialEvalContext::fromIP(ip, d);
		in.ShadingContext = ShadingContext::fromIP(tctx.Session.threadID(), ip);
		MaterialEvalOutput out;
		material->eval(in, out, tctx.Session);

		// Backward/reverse pdf
		in.Context = MaterialEvalContext::fromIP(ip, d, -ip.Ray.Direction); // Reverse
		MaterialPDFOutput pout;
		material->pdf(in, pout, tctx.Session);

		forwardPDF_S  = out.PDF_S[0];
		backwardPDF_S = pout.PDF_S[0];
		type		  = out.Type;
		return out.Weight;
	}

	void handleNEE(TracerContext& tctx,
				   const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, const CameraTraversalContext& current) const
	{
		const EntitySamplingInfo sampleInfo = { cameraIP.P, cameraIP.Surface.N };

		// Sample light
		LightSampleInput lsin;
		lsin.RND			= tctx.Session.random().get4D();
		lsin.WavelengthNM	= cameraIP.Ray.WavelengthNM;
		lsin.Point			= &cameraIP;
		lsin.SamplingInfo	= &sampleInfo;
		lsin.SamplePosition = true;
		LightSampleOutput lsout;
		const auto lsample = mLightSampler->sample(lsin, lsout, tctx.Session);
		const Light* light = lsample.first;
		if (PR_UNLIKELY(!light))
			return;

		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const float cameraRoulette	  = mCameraRR.probability(cameraPathLength);

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
		const SpectralBlob cameraW = extractMaterial(tctx, cameraIP, cameraMaterial, L, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);

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

		const bool isVisible	  = !tctx.Session.traceShadowRay(shadow, distance, light->entityID());
		const SpectralBlob lightW = isVisible ? lsout.Radiance : SpectralBlob::Zero();

		// Calculate contribution
		const SpectralBlob contrib = lightW * cameraW / (lsample.second * positionPDF_S);

		// Construct LPE path
		tctx.TmpPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			tctx.TmpPath.addToken(tctx.CameraPath.token(t2));

		tctx.TmpPath.addToken(cameraScatteringType);

		if (light->isInfinite())
			tctx.TmpPath.addToken(LightPathToken::Background());
		else
			tctx.TmpPath.addToken(LightPathToken::Emissive());

		// Splat
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib,
										  cameraIP.Ray, tctx.TmpPath);
	}

	void handleConnection(TracerContext& tctx,
						  const PathVertex& lightVertex,
						  const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, const CameraTraversalContext& current) const
	{
		Vector3f cD		  = (lightVertex.IP.P - cameraIP.P); // Camera Vertex -> Light Vertex
		const float dist2 = cD.squaredNorm();
		const float dist  = std::sqrt(dist2);
		cD /= dist;

		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const uint32 lightPathLength  = lightVertex.pathLength();

		// Extract necessary materials
		const IMaterial* lightMaterial = tctx.Session.getMaterial(lightVertex.IP.Surface.Geometry.MaterialID);
		PR_ASSERT(lightMaterial, "Expected valid material for light vertex");

		const float cameraRoulette = mCameraRR.probability(cameraPathLength);
		const float lightRoulette  = mLightRR.probability(lightPathLength);

		// Evaluate camera material
		float cameraForwardPDF_S;
		float cameraBackwardPDF_S;
		MaterialScatteringType cameraScatteringType;
		const SpectralBlob cameraW = extractMaterial(tctx, cameraIP, cameraMaterial, cD, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);
		cameraForwardPDF_S *= cameraRoulette;
		cameraBackwardPDF_S *= cameraRoulette;

		// Evaluate light material
		float lightForwardPDF_S;
		float lightBackwardPDF_S;
		MaterialScatteringType lightScatteringType; // Unused
		const SpectralBlob lightW = extractMaterial(tctx, lightVertex.IP, lightMaterial, -cD, lightForwardPDF_S, lightBackwardPDF_S, lightScatteringType);
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
		const bool isVisible = !tctx.Session.traceShadowRay(shadow, dist, lightVertex.IP.Surface.Geometry.EntityID);

		// Extract terms
		const SpectralBlob contrib = isVisible ? (lightW * Geometry * cameraW).eval() : SpectralBlob::Zero();

		// Construct LPE path
		tctx.TmpPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			tctx.TmpPath.addToken(tctx.CameraPath.token(t2));

		tctx.TmpPath.addToken(cameraScatteringType);

		for (size_t s2 = 0; s2 < lightPathLength; ++s2)
			tctx.TmpPath.addToken(tctx.LightPath.token(lightPathLength - 1 - s2));

		// Splat
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib, cameraIP.Ray, tctx.TmpPath);
	}

	// Handle case where camera ray directly hits emissive object
	void handleDirectHit(TracerContext& tctx, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, const CameraTraversalContext& current) const
	{
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;

		const IEmission* ems = tctx.Session.getEmission(cameraIP.Surface.Geometry.EmissionID);
		if (PR_UNLIKELY(!ems))
			return;

		// Cull emission if needed
		if (culling(-cameraIP.Surface.NdotV) <= PR_EPSILON)
			return;

		// Evaluate emission
		EmissionEvalInput ein;
		ein.Entity		   = cameraEntity;
		ein.ShadingContext = ShadingContext::fromIP(tctx.Session.threadID(), cameraIP);
		EmissionEvalOutput eout;
		ems->eval(ein, eout, tctx.Session);

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			tctx.Session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, eout.Radiance, cameraIP.Ray, tctx.CameraPath);
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
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, eout.Radiance, cameraIP.Ray, tctx.CameraPath);
	}

	// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const TracerContext& tctx, const CameraTraversalContext& current, const Ray& ray) const
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;

		// Evaluate radiance
		float dirPDF_S		  = 0;
		SpectralBlob radiance = SpectralBlob::Zero();
		for (auto light : tctx.Session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->eval(lin, lout, tctx.Session);

			radiance += lout.Radiance;
			dirPDF_S += lout.Direction_PDF_S;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			tctx.Session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, ray, tctx.CameraPath);
			return;
		}

		// Evaluate PDF (for now independent of concrete inf light)
		const float selProb = mLightSampler->pdfSelection(nullptr);
		auto posPDF			= mLightSampler->pdfPosition(nullptr);

		float posPDF_A = posPDF.Value;
		if (!posPDF.IsArea) {
			const float r = tctx.Session.context()->scene()->boundingSphere().radius();
			posPDF_A	  = IS::toArea(posPDF_A, r * r, 1);
		}

		posPDF_A *= selProb;
		dirPDF_S *= selProb;

		// Calculate MIS
		const float cameraMIS = mis_term(posPDF_A) * current.MIS_VCM + mis_term(dirPDF_S) * current.MIS_VC;
		const float mis		  = 1 / (1 + cameraMIS);

		// Splat
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, radiance, ray, tctx.CameraPath);
	}

private:
	const Options mOptions;
	const Walker mCameraWalker;
	const RussianRoulette mCameraRR;
	const Walker mLightWalker;
	const RussianRoulette mLightRR;
	const std::shared_ptr<LightSampler> mLightSampler;
};
} // namespace VCM
} // namespace PR