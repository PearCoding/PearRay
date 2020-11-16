#pragma once

#include "MIS.h"
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

template <bool UseMerging, bool HasInfLights, MISMode Mode = MM_Balance>
class Tracer {
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

		// Initial camera vertex
		CameraTraversalContext current = CameraTraversalContext{ SpectralBlob::Ones(), 0, 0, 0 };

		mCameraWalker.traverse(
			tctx.Session, initial_hit, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(tctx, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				if constexpr (HasInfLights) {
					tctx.CameraPath.addToken(LightPathToken::Background());
					handleInfLights(tctx, current, ray);
					tctx.CameraPath.popToken();
				} else {
					PR_UNUSED(ray);
				}
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

		if (light->isInfinite())
			tctx.LightPath.addToken(LightPathToken::Background());
		else
			tctx.LightPath.addToken(LightPathToken::Emissive());

		// Calculate PDF (TODO: Do not mix area & solid angle measure)
		float directPdfA   = 0; // PDF for NEE
		float emissionPdfS = 0; // PDF for Light emission
		if (light->isInfinite()) {
			directPdfA	 = lsout.Direction_PDF_S; // We lie a bit here
			emissionPdfS = directPdfA * lsout.Position_PDF.Value;
		} else {
			PR_ASSERT(lsout.Position_PDF.IsArea, "Expected area position pdf for area lights");
			directPdfA	 = lsout.Position_PDF.Value;
			emissionPdfS = directPdfA * lsout.Direction_PDF_S;
		}
		directPdfA *= lsample.second;
		emissionPdfS *= lsample.second;

		if (emissionPdfS <= PR_EPSILON) // Don't even try
			return light;

		// Setup light path context
		LightTraversalContext current = LightTraversalContext{ lsout.Radiance, 0, 0, 0, !light->isInfinite() };

		current.Throughput /= emissionPdfS;
		current.MIS_VCM = mis_term<Mode>(directPdfA / emissionPdfS);
		if (light->hasDeltaDistribution())
			current.MIS_VC = 0;
		else
			current.MIS_VC = mis_term<Mode>(lsout.CosLight / emissionPdfS);

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
										IEntity* entity, IMaterial* material, BaseTraversalContext& current) const
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
			current.MIS_VC *= mis_term<Mode>(NdotL);
			if constexpr (UseMerging)
				current.MIS_VM = 0;
			current.MIS_VCM = 0;
		} else {
			current.MIS_VC = mis_term<Mode>(NdotL / backwardPDF_S) * (current.MIS_VC * mis_term<Mode>(backwardPDF_S) + current.MIS_VCM);
			if constexpr (UseMerging)
				current.MIS_VM = 0; // TODO
			current.MIS_VCM = mis_term<Mode>(1 / backwardPDF_S);
		}

		if (NdotL <= PR_EPSILON) // Do not bother if laying flat on the sampling plane
			return {};

		if (backwardPDF_S <= PR_EPSILON || forwardPDF_S <= PR_EPSILON) // Catch this case, or everything will explode
			return {};

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

		return std::make_optional(ip.nextRay(L, rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
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
			current.MIS_VCM *= mis_term<Mode>(ip.Depth2);

		if (ip.isAtSurface()) {
			const float ndotv = std::abs(ip.Surface.NdotV);
			if (ndotv <= PR_EPSILON)
				return {};

			const float f = 1 / mis_term<Mode>(std::abs(ndotv));
			current.MIS_VCM *= f;
			current.MIS_VC *= f;
			if constexpr (UseMerging)
				current.MIS_VM *= f;
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
			vertex.MIS_VM	  = current.MIS_VM;
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
		current.MIS_VCM *= mis_term<Mode>(ip.Depth2);
		if (ip.isAtSurface()) {
			const float ndotv = std::abs(ip.Surface.NdotV);
			if (ndotv <= PR_EPSILON)
				return {};

			const float f = 1 / mis_term<Mode>(ndotv);
			current.MIS_VCM *= f;
			current.MIS_VC *= f;
			if constexpr (UseMerging)
				current.MIS_VM *= f;
		}

		float sumMIS = 0.0f;

		// Handle light emission (c0t)
		if (entity->hasEmission()) {
			tctx.CameraPath.addToken(LightPathToken::Emissive());
			handleDirectHit(tctx, ip, entity, current, sumMIS);
			tctx.CameraPath.popToken();
		}

		// If there is no material to scatter from, give up
		if (PR_UNLIKELY(!material))
			return {};

		if (!material->hasDeltaDistribution()) {
			// c1t
			handleNEE(tctx, ip, material, current, sumMIS);
			// cst
			for (const auto& vertex : tctx.LightVertices)
				handleConnection(tctx, vertex, ip, material, current, sumMIS);
		}

		// Some terms are ignored due to wrong pdf, cosine etc. Compensate for them here
		/*PR_ASSERT(sumMIS <= 1.0f, "Expected full MIS term being between 0 and 1!");
		if (sumMIS < 1.0f) {
			tctx.CameraPath.addToken(LightPathToken::Background());
			tctx.Session.pushSpectralFragment(SpectralBlob(1 - sumMIS), current.Throughput, SpectralBlob::Zero(),
											  ip.Ray, tctx.CameraPath);
			tctx.CameraPath.popToken();
		}*/

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

		//if (ip.Ray.Flags & RF_Monochrome) {
		forwardPDF_S  = out.PDF_S[0];
		backwardPDF_S = pout.PDF_S[0];
		/*} else {
			forwardPDF_S  = out.PDF_S.sum();
			backwardPDF_S = pout.PDF_S.sum();
		}*/
		type = out.Type;
		return out.Weight;
	}

	void handleNEE(TracerContext& tctx,
				   const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, CameraTraversalContext& current, float& sumMIS) const
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
		const float cosC	  = std::abs(L.dot(cameraIP.Surface.N));
		const float cosL	  = std::abs(lsout.CosLight);
		const float Geometry  = cosC * cosL / sqrD;
		const bool isFeasible = Geometry > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		if (!isFeasible) // MIS is zero
			return;

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
		float posPdfS = lsout.Position_PDF.Value;
		if (lsout.Position_PDF.IsArea)
			posPdfS = IS::toSolidAngle(posPdfS, sqrD, cosL);

		float directPdfS   = 0; // PDF for NEE
		float emissionPdfS = 0; // PDF for Light emission
		if (light->isInfinite()) {
			directPdfS	 = lsout.Direction_PDF_S;
			emissionPdfS = directPdfS * posPdfS;
		} else {
			directPdfS	 = posPdfS;
			emissionPdfS = directPdfS * light->pdfDirection(L, lsout.CosLight);
		}

		if (directPdfS <= PR_EPSILON)
			return;

		// Calculate MIS
		const float lightMIS  = mis_term<Mode>(cameraForwardPDF_S / (lsample.second * directPdfS));
		const float cameraMIS = mis_term<Mode>(emissionPdfS * cosC / (directPdfS * cosL))
								* (current.MIS_VCM + current.MIS_VC * mis_term<Mode>(cameraBackwardPDF_S));
		const float mis = 1 / (1 + lightMIS + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Ray shadow	 = cameraIP.nextRay(L, RF_Shadow, SHADOW_RAY_MIN, distance);

		const bool isVisible	  = lsout.CosLight > 0.0f && !tctx.Session.traceShadowRay(shadow, distance);
		const SpectralBlob lightW = lsout.Radiance;

		// Calculate contribution
		const SpectralBlob contrib = isVisible ? (lightW * cameraW / (lsample.second * directPdfS)).eval() : SpectralBlob::Zero();

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
		sumMIS += mis;
		//PR_ASSERT(sumMIS <= 1.0f, "Expected full MIS term being between 0 and 1! 4353452352");
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib,
										  cameraIP.Ray, tctx.TmpPath);
	}

	void handleConnection(TracerContext& tctx,
						  const PathVertex& lightVertex,
						  const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, CameraTraversalContext& current, float& sumMIS) const
	{
		Vector3f cD		  = (lightVertex.IP.P - cameraIP.P); // Camera Vertex -> Light Vertex
		const float dist2 = cD.squaredNorm();

		if (dist2 <= DISTANCE_EPS) // If we are that close, we should merge instead
			return;

		const float dist = std::sqrt(dist2);
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
		const float cosC	  = std::abs(cD.dot(cameraIP.Surface.N));
		const float cosL	  = std::abs(cD.dot(lightVertex.IP.Surface.N));
		const float Geometry  = cosL / dist2; // cosC already included in material
		const bool isFeasible = cosC * Geometry > GEOMETRY_EPS;
		if (!isFeasible)
			return;

		// To Area
		const float cameraForwardPDF_A = IS::toArea(cameraForwardPDF_S, dist2, cosL);
		const float lightForwardPDF_A  = IS::toArea(lightForwardPDF_S, dist2, cosC);

		// Calculate MIS
		const float cameraMIS = mis_term<Mode>(cameraForwardPDF_A) * (lightVertex.MIS_VCM + lightVertex.MIS_VC * mis_term<Mode>(lightBackwardPDF_S));
		const float lightMIS  = mis_term<Mode>(lightForwardPDF_A) * (current.MIS_VCM + current.MIS_VC * mis_term<Mode>(cameraBackwardPDF_S));
		const float mis		  = 1 / (1 + lightMIS + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Construct ray
		const Ray shadow = cameraIP.nextRay(cD, RF_Shadow, SHADOW_RAY_MIN, dist);

		// Extract visible and geometry term
		const bool isVisible = !tctx.Session.traceShadowRay(shadow, dist);

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
		sumMIS += mis;
		//PR_ASSERT(sumMIS <= 1.0f, "Expected full MIS term being between 0 and 1! fdasdsdasd");
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, contrib, cameraIP.Ray, tctx.TmpPath);
	}

	// Handle case where camera ray directly hits emissive object
	void handleDirectHit(TracerContext& tctx, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, CameraTraversalContext& current, float& sumMIS) const
	{
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;

		const IEmission* ems = tctx.Session.getEmission(cameraIP.Surface.Geometry.EmissionID);
		if (PR_UNLIKELY(!ems))
			return;

		// Cull emission if needed
		const float cosL = -cameraIP.Surface.NdotV;
		if (std::abs(cosL) <= PR_EPSILON)
			return;

		// Evaluate emission
		SpectralBlob radiance;
		if (cosL < 0.0f) {
			radiance = SpectralBlob::Zero();
		} else {
			EmissionEvalInput ein;
			ein.Entity		   = cameraEntity;
			ein.ShadingContext = ShadingContext::fromIP(tctx.Session.threadID(), cameraIP);
			EmissionEvalOutput eout;
			ems->eval(ein, eout, tctx.Session);
			radiance = eout.Radiance;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			tctx.Session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, cameraIP.Ray, tctx.CameraPath);
			return;
		}

		// Evaluate PDF
		const float selProb = mLightSampler->pdfEntitySelection(cameraEntity);
		auto posPDF			= mLightSampler->pdfPosition(cameraEntity, cameraIP.P);
		PR_ASSERT(posPDF.IsArea, "Area lights should return pdfs respective to area!");

		const float dirPDF_S = mLightSampler->pdfDirection(cameraIP.Ray.Direction, cameraEntity) * selProb;
		const float posPDF_A = posPDF.Value * selProb;

		// Calculate MIS
		const float cameraMIS = mis_term<Mode>(posPDF_A) * current.MIS_VCM + mis_term<Mode>(dirPDF_S) * current.MIS_VC;
		const float mis		  = 1 / (1 + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		sumMIS += mis;
		tctx.Session.pushSpectralFragment(SpectralBlob(mis), current.Throughput, radiance, cameraIP.Ray, tctx.CameraPath);
	}

	// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const TracerContext& tctx, CameraTraversalContext& current, const Ray& ray) const
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;

		// Evaluate radiance
		float misDenom		  = 0;
		SpectralBlob radiance = SpectralBlob::Zero();
		for (auto light : mLightSampler->infiniteLights()) {
			if (light->hasDeltaDistribution())
				continue;

			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->asInfiniteLight()->eval(lin, lout, tctx.Session);

			if (lout.Direction_PDF_S <= PR_EPSILON)
				continue;

			radiance += lout.Radiance;

			// Evaluate PDF
			const float selProb = mLightSampler->pdfLightSelection(light);
			auto posPDF			= mLightSampler->pdfPosition(nullptr, Vector3f::Zero()); // TODO: Implement pdf evaluation for infinite lights

			float posPDF_A = posPDF.Value;
			if (!posPDF.IsArea) {
				const float r = tctx.Session.context()->scene()->boundingSphere().radius();
				posPDF_A	  = IS::toArea(posPDF_A, r * r, 1);
			}

			posPDF_A *= selProb;
			const float dirPDF_S = lout.Direction_PDF_S * selProb;

			// Calculate MIS
			const float cameraMIS = mis_term<Mode>(posPDF_A) * current.MIS_VCM + mis_term<Mode>(dirPDF_S) * current.MIS_VC;
			misDenom += cameraMIS;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			tctx.Session.pushSpectralFragment(SpectralBlob::Ones(), current.Throughput, radiance, ray, tctx.CameraPath);
			return;
		}

		const float mis = 1 / (1 + misDenom);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

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