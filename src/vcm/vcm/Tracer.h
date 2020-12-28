#pragma once

#include "MIS.h"
#include "Options.h"
#include "PathVertex.h"
#include "RussianRoulette.h"
#include "TracerIterationContext.h"
#include "TracerThreadContext.h"
#include "TraversalContext.h"
#include "Utils.h"
#include "Walker.h"
#include "Wavelength.h"
#include "container/Interval.h"
#include "renderer/RenderTileSession.h"

#include "Logger.h"

namespace PR {
namespace VCM {

// This is based on http://www.smallvcm.com/
template <bool UseMerging, MISMode Mode = MM_Balance>
class Tracer {
public:
	using IterationContext = TracerIterationContext<UseMerging, Mode>;
	using ThreadContext	   = TracerThreadContext;

	inline Tracer(const Options& options, const std::shared_ptr<LightSampler>& lightSampler)
		: mOptions(options)
		, mCameraWalker(options.MaxCameraRayDepthHard)
		, mCameraRR(options.MaxCameraRayDepthSoft)
		, mLightWalker(options.MaxLightRayDepthHard)
		, mLightRR(options.MaxLightRayDepthSoft)
		, mLightSampler(lightSampler)
		, mLightPathSlice(options.MaxLightRayDepthHard + 1)
		, mLightPathCounter(0)
		, mLightVertices(UseMerging ? options.MaxLightSamples * mLightPathSlice : 1)
		, mLightPathSize(UseMerging ? options.MaxLightSamples : 1)
		, mLightPathWavelengthSortMap(mLightPathSize.size())
		, mLightPathBuffer(UseMerging ? options.MaxLightSamples * LightPath::packedSizeRequirement(mLightPathSlice + 1) : 1)
	{
	}

	inline const Options& options() const { return mOptions; }

	inline void traceCameraPath(IterationContext& tctx, const IntersectionPoint& initial_hit,
								IEntity* entity, IMaterial* material) const
	{
		PR_PROFILE_THIS;

		tctx.Session.pushSPFragment(initial_hit, tctx.ThreadContext.CameraPath);

		if constexpr (UseMerging) {
			if (PR_UNLIKELY(mLightPathCounter == 0))
				return;
		}

		// Initial camera vertex
		CameraTraversalContext current;

		if constexpr (UseMerging) {
			// Select light path to work with
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				current.LightPathID		 = pickClosestLightPath(initial_hit.Ray.WavelengthNM[i]);
				const PathVertex* vertex = lightVertex(current.LightPathID, 0);
				if (!checkWavelengthSupport(initial_hit.Ray.WavelengthNM, initial_hit.Ray.Flags & RF_Monochrome,
											vertex->IP.Ray.WavelengthNM, vertex->IP.Ray.Flags & RF_Monochrome,
											current.Permutation))
					break; // Nothing found
			}

			const PathVertex* finalVertex = lightVertex(current.LightPathID, 0);
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				current.WavelengthFilter[i] = wavelengthFilter(
					initial_hit.Ray.WavelengthNM[i],
					finalVertex->IP.Ray.WavelengthNM[i]);

			// Setup light path
			const size_t packSize = LightPath::packedSizeRequirement(mLightPathSlice + 1);
			tctx.ThreadContext.LightPath.reset();
			tctx.ThreadContext.LightPath.addFromPacked(&mLightPathBuffer[current.LightPathID * packSize],
													   packSize);
		}

		// Initial camera vertex
		mCameraWalker.traverse(
			tctx.Session, initial_hit, entity, material,
			[&](const IntersectionPoint& ip, IEntity* entity2, IMaterial* material2) -> std::optional<Ray> {
				return handleCameraVertex(tctx, ip,
										  entity2, material2, current);
			},
			[&](const Ray& ray) {
				tctx.ThreadContext.CameraPath.addToken(LightPathToken::Background());
				handleInfLights(tctx, current, ray);
				tctx.ThreadContext.CameraPath.popToken();
			});
	}

	// First light vertex
	const Light* traceLightPath(IterationContext& tctx, const SpectralBlob& wvl)
	{
		PR_PROFILE_THIS;
		if constexpr (UseMerging)
			PR_ASSERT(mLightPathCounter < mOptions.MaxLightSamples, "Do not call trace more than the expected number of light samples");

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

		tctx.ThreadContext.LightPath.reset();
		if (light->isInfinite())
			tctx.ThreadContext.LightPath.addToken(LightPathToken::Background());
		else
			tctx.ThreadContext.LightPath.addToken(LightPathToken::Emissive());

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
		LightTraversalContext current;
		current.Throughput	  = lsout.Radiance;
		current.LightPathID	  = mLightPathCounter.fetch_add(1);
		current.LightPathSize = 0;
		current.IsFiniteLight = !light->isInfinite();

		// After acquiring the path, make sure its initialized to the empty size
		if constexpr (UseMerging)
			mLightPathSize[current.LightPathID] = 0;

		current.Throughput /= emissionPdfS;
		if (current.Throughput.isZero(PR_EPSILON)) // Don't even try
			return light;

		current.MIS_VCM = mis_term<Mode>(directPdfA / emissionPdfS);
		if (light->hasDeltaDistribution())
			current.MIS_VC = 0;
		else
			current.MIS_VC = mis_term<Mode>(lsout.CosLight / emissionPdfS);

		current.MIS_VM = current.MIS_VC * tctx.MISVCWeightFactor();

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

		// Make sure the end of the path is given
		if constexpr (UseMerging) {
			mLightPathSize[current.LightPathID] = current.LightPathSize;
			const size_t packSize				= LightPath::packedSizeRequirement(mLightPathSlice + 1);
			tctx.ThreadContext.LightPath.toPacked(&mLightPathBuffer[current.LightPathID * packSize],
												  packSize);
		}

		return light;
	}

	// We assume only registration is done within multiple threads
	// You may ignore registration of thread contexts if UseMerging is false
	inline void registerThreadContext()
	{
		std::lock_guard<std::mutex> guard(mMutex);
		mThreadContexts.emplace_back(std::make_unique<ThreadContext>(mOptions));
	}

	inline void registerThreadContext(const BoundingBox& bbox, float sceneGridDelta)
	{
		std::lock_guard<std::mutex> guard(mMutex);
		mThreadContexts.emplace_back(std::make_unique<ThreadContext>(bbox, sceneGridDelta, mOptions));
	}

	// While accessing thread contexts, no new registration should occur!
	inline ThreadContext& threadContext(size_t id) { return *mThreadContexts[id].get(); }
	inline size_t threadContextCount() { return mThreadContexts.size(); }

	// Make sure only one thread is calling this function!
	inline void resetLights()
	{
		mLightPathCounter = 0;
		if (mLightMap)
			mLightMap->reset();
	}

	// Make sure only one thread is calling this function!
	/// Construct search grid for this particular thread context
	inline void setupSearchGrid(const BoundingBox& bbox, float sceneGatherRadius)
	{
		mMutex.lock();
		if (!mLightMap)
			mLightMap = std::make_unique<PathVertexMap>(bbox, sceneGatherRadius, mLightVertices);
		mMutex.unlock();

		const size_t amount = mLightPathCounter;
		for (size_t i = 0; i < amount; ++i) // This can be parallelized
			mLightMap->storeUnsafe(i);
	}

	inline void setupWavelengthSelector()
	{
		// Fill array with increasing numbers
		const auto end = mLightPathWavelengthSortMap.begin() + mLightPathCounter;
		std::iota(mLightPathWavelengthSortMap.begin(), end, 0);

		// Sort based on the hero wavelength
		std::sort(mLightPathWavelengthSortMap.begin(), end,
				  [this](size_t a, size_t b) {
					  const PathVertex* v1 = lightVertex(a, 0);
					  const PathVertex* v2 = lightVertex(b, 0);
					  return v1->IP.Ray.WavelengthNM[0] < v2->IP.Ray.WavelengthNM[0];
				  });
	}

	inline uint32 pickClosestLightPath(float wvl) const
	{
		if constexpr (!UseMerging)
			PR_ASSERT(false, "Picking closest lightpath is disabled for bdpt, as it has its light path already associated with the camera path");

		// Select closest pick based on the given wavelength
		int pick = Interval::binary_search(mLightPathCounter, [&](size_t index) {
			const PathVertex* v1 = lightVertex(mLightPathWavelengthSortMap[index], 0);
			return v1->IP.Ray.WavelengthNM[0] <= wvl;
		});

		if (pick < 0 || static_cast<size_t>(pick) >= mLightPathCounter)
			return 0;
		return pick;
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
	std::optional<Ray> handleScattering(IterationContext& tctx, const IntersectionPoint& ip,
										IEntity* entity, IMaterial* material, BaseTraversalContext& current) const
	{
		PR_ASSERT(entity, "Expected valid entity");

		auto& rnd		= tctx.Session.random();
		LightPath& path = IsCamera ? tctx.ThreadContext.CameraPath : tctx.ThreadContext.LightPath;

		// Russian roulette
		const auto roulette = checkRoulette<IsCamera>(tctx.Session.random(), ip.Ray.IterationDepth + 1);
		if (!roulette.has_value())
			return {};

		const float scatProb = roulette.value();

		// TODO: Add volume support
		if (!material)
			return {};

		// Sample Material
		MaterialSampleInput sin(ip, tctx.Session.threadID(), rnd);
		MaterialSampleOutput sout;
		material->sample(sin, sout, tctx.Session);

		path.addToken(sout.Type);

		const Vector3f L   = sout.globalL(ip);
		const bool isDelta = sout.isDelta();

		if (sout.Weight.isZero()) {
			path.popToken();
			return {};
		}

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
			const float misNdotL = mis_term<Mode>(NdotL);
			current.MIS_VC *= misNdotL;
			if constexpr (UseMerging)
				current.MIS_VM *= misNdotL;
			current.MIS_VCM = 0;
		} else {
			const float misNdotL = mis_term<Mode>(NdotL / forwardPDF_S);
			current.MIS_VC		 = misNdotL * (current.MIS_VC * mis_term<Mode>(backwardPDF_S) + current.MIS_VCM + tctx.MISVMWeightFactor());
			if constexpr (UseMerging)
				current.MIS_VM = misNdotL * (current.MIS_VM * mis_term<Mode>(backwardPDF_S) + current.MIS_VCM * tctx.MISVCWeightFactor() + 1.0f);
			current.MIS_VCM = mis_term<Mode>(1 / forwardPDF_S);
		}

		// Do not bother if laying flat on the sampling plane
		if (NdotL <= PR_EPSILON) {
			path.popToken();
			return {};
		}

		// Catch this case, or everything will explode
		if (backwardPDF_S <= PR_EPSILON || forwardPDF_S <= PR_EPSILON) {
			path.popToken();
			return {};
		}

		// Update throughput
		current.Throughput *= sout.Weight / forwardPDF_S;
		if constexpr (!IsCamera)
			current.Throughput *= correctShadingNormalForLight(-ip.Ray.Direction, L, ip.Surface.N, ip.Surface.Geometry.N);

		if (sout.isHeroCollapsing())
			current.Throughput *= SpectralBlobUtils::HeroOnly();

		if (current.Throughput.isZero(PR_EPSILON)) {
			path.popToken();
			return {};
		}

		// Setup ray flags
		int rflags = RF_Bounce;
		if (sout.isHeroCollapsing())
			rflags |= RF_Monochrome;

		return std::make_optional(ip.nextRay(L, rflags, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX));
	}

	std::optional<Ray> handleLightVertex(IterationContext& tctx, const IntersectionPoint& ip,
										 IEntity* entity, IMaterial* material, LightTraversalContext& current)
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
		if (!material->hasOnlyDeltaDistribution()) {
			PathVertex vertex;
			vertex.IP		  = ip;
			vertex.Throughput = current.Throughput;
			vertex.MIS_VCM	  = current.MIS_VCM;
			vertex.MIS_VC	  = current.MIS_VC;
			vertex.MIS_VM	  = current.MIS_VM;

			if constexpr (UseMerging) {
				mLightVertices[current.LightPathID * mLightPathSlice + current.LightPathSize] = std::move(vertex);
				current.LightPathSize++;
			} else {
				tctx.ThreadContext.BDPTLightVertices.emplace_back(vertex);
			}
		}
		// Our implementation does not support direct camera hits (no C_s0)

		return handleScattering<false>(tctx, ip, entity, material, current);
	}

	std::optional<Ray> handleCameraVertex(IterationContext& tctx, const IntersectionPoint& ip,
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

		// Handle light emission (c0t)
		if (entity->hasEmission()) {
			tctx.ThreadContext.CameraPath.addToken(LightPathToken::Emissive());
			handleDirectHit(tctx, ip, entity, current);
			tctx.ThreadContext.CameraPath.popToken();
		}

		// If there is no material to scatter from, give up
		if (PR_UNLIKELY(!material))
			return {};

		if (!material->hasOnlyDeltaDistribution()) {
			// c1t
			handleNEE(tctx, ip, material, current);
			// cst
			if constexpr (UseMerging) {
				const auto range = lightPath(current.LightPathID);
				for (auto it = range.first; it != range.second; ++it)
					handleConnection(tctx, *it, ip, material, current);
			} else {
				for (const auto& vertex : tctx.ThreadContext.BDPTLightVertices)
					handleConnection(tctx, vertex, ip, material, current);
			}

			// VM
			if constexpr (UseMerging)
				handleMerging(tctx, ip, material, current);
		}

		return handleScattering<true>(tctx, ip, entity, material, current);
	}

	inline SpectralBlob extractMaterial(const IterationContext& tctx,
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

		if (ip.Ray.Flags & RF_Monochrome) {
			forwardPDF_S  = out.PDF_S[0];
			backwardPDF_S = pout.PDF_S[0];
			out.Weight *= SpectralBlobUtils::HeroOnly();
		} else {
			forwardPDF_S  = out.PDF_S.sum();
			backwardPDF_S = pout.PDF_S.sum();
		}
		type = out.Type;
		return out.Weight;
	}

	void handleNEE(IterationContext& tctx,
				   const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, CameraTraversalContext& current) const
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
		const float sqrD	   = (lsout.LightPosition - cameraIP.P).squaredNorm();
		const Vector3f L	   = lsout.Outgoing;
		const float cosCS	   = L.dot(cameraIP.Surface.N);
		const float cosC	   = std::abs(cosCS);
		const float cosL	   = std::abs(lsout.CosLight);
		const bool front2front = cosCS >= 0.0f && lsout.CosLight >= 0.0f;
		const float Geometry   = cosC * cosL / sqrD;
		const bool isFeasible  = Geometry > GEOMETRY_EPS && sqrD > DISTANCE_EPS;

		if (!isFeasible) // MIS is zero
			return;

		// Evaluate camera material
		float cameraForwardPDF_S;
		float cameraBackwardPDF_S;
		MaterialScatteringType cameraScatteringType;
		const SpectralBlob cameraW = extractMaterial(tctx, cameraIP, cameraMaterial, L, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);
		// Its impossible to sample the bsdf, so skip it
		if (cameraForwardPDF_S <= PR_EPSILON || cameraBackwardPDF_S <= PR_EPSILON)
			return;

		if (!light->hasDeltaDistribution())
			cameraForwardPDF_S *= cameraRoulette;
		else
			cameraForwardPDF_S = 0;
		cameraBackwardPDF_S *= cameraRoulette;

		// Calculate direct and emission pdf (TODO: We really need a better abstraction)
		float directPdfS   = 0; // PDF for NEE
		float emissionPdfS = 0; // PDF for Light emission
		if (light->isInfinite()) {
			float posPdfA = lsout.Position_PDF.Value;
			if (!lsout.Position_PDF.IsArea)
				posPdfA = IS::toArea(posPdfA, sqrD, cosL);
			directPdfS	 = lsout.Direction_PDF_S;
			emissionPdfS = directPdfS * posPdfA; // We lie here
		} else {
			float posPdfS = lsout.Position_PDF.Value;
			float posPdfA = lsout.Position_PDF.Value;
			if (lsout.Position_PDF.IsArea)
				posPdfS = IS::toSolidAngle(posPdfA, sqrD, cosL);
			else
				posPdfA = IS::toArea(posPdfS, sqrD, cosL);
			directPdfS	 = posPdfS;
			emissionPdfS = posPdfA * light->pdfDirection(L, lsout.CosLight);
		}

		if (directPdfS <= PR_EPSILON || emissionPdfS <= PR_EPSILON)
			return;

		// Calculate MIS
		const float lightMIS  = mis_term<Mode>(cameraForwardPDF_S / (lsample.second * directPdfS));
		const float cameraMIS = mis_term<Mode>(emissionPdfS * cosC / (directPdfS * cosL))
								* (tctx.MISVMWeightFactor() + current.MIS_VCM + current.MIS_VC * mis_term<Mode>(cameraBackwardPDF_S));
		const float mis = 1 / (1 + lightMIS + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Trace shadow ray
		const float distance = light->isInfinite() ? PR_INF : std::sqrt(sqrD);
		const Ray shadow	 = cameraIP.nextRay(L, RF_Shadow, SHADOW_RAY_MIN, distance);

		const bool isVisible	  = front2front && !tctx.Session.traceShadowRay(shadow, distance);
		const SpectralBlob lightW = lsout.Radiance;

		// Calculate contribution
		const SpectralBlob contrib = isVisible ? (lightW * cameraW / (lsample.second * directPdfS)).eval() : SpectralBlob::Zero();

		// Construct LPE path
		tctx.ThreadContext.TmpPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			tctx.ThreadContext.TmpPath.addToken(tctx.ThreadContext.CameraPath.token(t2));

		tctx.ThreadContext.TmpPath.addToken(cameraScatteringType);

		if (light->isInfinite()) {
			tctx.Session.tile()->statistics().addBackgroundHitCount();
			tctx.ThreadContext.TmpPath.addToken(LightPathToken::Background());
		} else {
			tctx.Session.tile()->statistics().addEntityHitCount();
			tctx.ThreadContext.TmpPath.addToken(LightPathToken::Emissive());
		}

		// Splat
		tctx.Session.pushSpectralFragment(mis, current.Throughput, contrib,
										  cameraIP.Ray, tctx.ThreadContext.TmpPath);
	}

	void handleConnection(IterationContext& tctx,
						  const PathVertex& lightVertex,
						  const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, CameraTraversalContext& current) const
	{
		// Calculate geometry term
		Vector3f cD			  = (lightVertex.IP.P - cameraIP.P); // Camera Vertex -> Light Vertex
		const float dist2	  = cD.squaredNorm();
		const float cosC	  = std::abs(cD.dot(cameraIP.Surface.N));
		const float cosL	  = std::abs(cD.dot(lightVertex.IP.Surface.N));
		const float Geometry  = cosL / dist2; // cosC already included in material
		const bool isFeasible = cosC * Geometry > GEOMETRY_EPS && dist2 > DISTANCE_EPS;
		if (!isFeasible)
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
		// Its impossible to sample the bsdf, so skip it
		if (cameraForwardPDF_S <= PR_EPSILON || cameraBackwardPDF_S <= PR_EPSILON)
			return;

		cameraForwardPDF_S *= cameraRoulette;
		cameraBackwardPDF_S *= cameraRoulette;

		// Evaluate light material
		float lightForwardPDF_S;
		float lightBackwardPDF_S;
		MaterialScatteringType lightScatteringType; // Unused
		SpectralBlob lightW = extractMaterial(tctx, lightVertex.IP, lightMaterial, -cD, lightForwardPDF_S, lightBackwardPDF_S, lightScatteringType);
		// Its impossible to sample the bsdf, so skip it
		if (lightForwardPDF_S <= PR_EPSILON || lightBackwardPDF_S <= PR_EPSILON)
			return;

		lightForwardPDF_S *= lightRoulette;
		lightBackwardPDF_S *= lightRoulette;

		// We apply permutation to make sure each wavelength fits its counter part
		if constexpr (UseMerging) {
			SpectralBlob lightSwizzle;
			PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				lightSwizzle[i] = lightW[current.Permutation[i]];
			lightW = lightSwizzle * current.WavelengthFilter;
		}

		// Calculate connection weight
		const SpectralBlob connectionW = lightW * cameraW;
		const bool worthACheck		   = !connectionW.isZero();

		// To Area
		const float cameraForwardPDF_A = IS::toArea(cameraForwardPDF_S, dist2, cosL);
		const float lightForwardPDF_A  = IS::toArea(lightForwardPDF_S, dist2, cosC);

		// Calculate MIS
		const float cameraMIS = mis_term<Mode>(cameraForwardPDF_A) * (tctx.MISVMWeightFactor() + lightVertex.MIS_VCM + lightVertex.MIS_VC * mis_term<Mode>(lightBackwardPDF_S));
		const float lightMIS  = mis_term<Mode>(lightForwardPDF_A) * (tctx.MISVMWeightFactor() + current.MIS_VCM + current.MIS_VC * mis_term<Mode>(cameraBackwardPDF_S));
		const float mis		  = 1 / (1 + lightMIS + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Construct ray
		const Ray shadow = cameraIP.nextRay(cD, RF_Shadow, SHADOW_RAY_MIN, dist);

		// Extract visible and geometry term
		const bool isVisible = worthACheck && !tctx.Session.traceShadowRay(shadow, dist);

		// Extract terms
		const SpectralBlob contrib = isVisible ? (connectionW * Geometry).eval() : SpectralBlob::Zero();

		// Construct LPE path
		tctx.ThreadContext.TmpPath.reset();
		for (size_t t2 = 0; t2 < cameraPathLength; ++t2)
			tctx.ThreadContext.TmpPath.addToken(tctx.ThreadContext.CameraPath.token(t2));

		tctx.ThreadContext.TmpPath.addToken(cameraScatteringType);

		for (size_t s2 = 0; s2 < lightPathLength; ++s2)
			tctx.ThreadContext.TmpPath.addToken(tctx.ThreadContext.LightPath.token(lightPathLength - 1 - s2));

		// Splat
		tctx.Session.pushSpectralFragment(mis, current.Throughput, contrib, cameraIP.Ray, tctx.ThreadContext.TmpPath);
	}

	void handleMerging(IterationContext& tctx, const IntersectionPoint& cameraIP, const IMaterial* cameraMaterial, CameraTraversalContext& current) const
	{
		const uint32 cameraPathLength = cameraIP.Ray.IterationDepth + 1;
		const float cameraRoulette	  = mCameraRR.probability(cameraPathLength);

		// Construct query
		QuerySphere query;
		query.Center		= cameraIP.P;
		query.Normal		= cameraIP.Surface.N;
		query.Distance2		= tctx.Radius2;
		query.SqueezeWeight = mOptions.SqueezeWeight2;

		// Accumulation Function
		const auto accumFunc = [&](SpectralBlob&, const PathVertex& vertex, float d2) {
			const float f = kernel(d2 / query.Distance2);

			const SpectralBlob power = vertex.Throughput * f;
			SpectralBlob lightW		 = SpectralBlob::Zero();
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				for (size_t j = 0; j < PR_SPECTRAL_BLOB_SIZE; ++j)
					lightW[i] += wavelengthFilter(cameraIP.Ray.WavelengthNM[i], vertex.IP.Ray.WavelengthNM[j]) * power[j];

			if (lightW.isZero(GATHER_EPS))
				return;

			const Vector3f L		  = -vertex.IP.Ray.Direction;
			const float lightRoulette = mLightRR.probability(vertex.IP.Ray.IterationDepth + 1);

			// Apply shading
			float cameraForwardPDF_S; // == lightBackwardPDF_S
			float cameraBackwardPDF_S;
			MaterialScatteringType cameraScatteringType; // Ignore
			const SpectralBlob cameraW = extractMaterial(tctx, cameraIP, cameraMaterial, L, cameraForwardPDF_S, cameraBackwardPDF_S, cameraScatteringType);
			// Its impossible to sample the bsdf, so skip it
			if (cameraForwardPDF_S <= PR_EPSILON || cameraBackwardPDF_S <= PR_EPSILON)
				return;

			// Apply russian roulette
			cameraForwardPDF_S *= cameraRoulette;
			cameraBackwardPDF_S *= lightRoulette;

			// Calculate MIS
			const float misLight  = vertex.MIS_VCM * tctx.MISVCWeightFactor() + vertex.MIS_VM * mis_term<Mode>(cameraForwardPDF_S);
			const float misCamera = current.MIS_VCM * tctx.MISVCWeightFactor() + current.MIS_VM * mis_term<Mode>(cameraBackwardPDF_S);

			const float mis = 1 / (1 + misLight + misCamera);

			tctx.Session.pushSpectralFragment(mis, current.Throughput, lightW * cameraW, cameraIP.Ray, tctx.ThreadContext.CameraPath);
		};

		// Start gathering
		tctx.ThreadContext.CameraPath.addToken(LightPathToken::Emissive());

		size_t found = 0;
		mLightMap->estimateDome(query, accumFunc, found);

		tctx.ThreadContext.CameraPath.popToken();
	}

	/// Handle case where camera ray directly hits emissive object
	void handleDirectHit(IterationContext& tctx, const IntersectionPoint& cameraIP,
						 const IEntity* cameraEntity, CameraTraversalContext& current) const
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
			tctx.Session.pushSpectralFragment(1, current.Throughput, radiance, cameraIP.Ray, tctx.ThreadContext.CameraPath);
			return;
		}

		// Evaluate PDF
		const float selProb = mLightSampler->pdfEntitySelection(cameraEntity);
		auto posPDF			= mLightSampler->pdfPosition(cameraEntity, cameraIP.P);
		PR_ASSERT(posPDF.IsArea, "Area lights should return pdfs respective to area!");

		const float directPdfA	 = posPDF.Value * selProb;
		const float emissivePdfS = mLightSampler->pdfDirection(cameraIP.Ray.Direction, cameraEntity, cosL) * directPdfA;

		// Calculate MIS
		const float cameraMIS = mis_term<Mode>(directPdfA) * current.MIS_VCM + mis_term<Mode>(emissivePdfS) * current.MIS_VC;
		const float mis		  = 1 / (1 + cameraMIS);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		tctx.Session.pushSpectralFragment(mis, current.Throughput, radiance, cameraIP.Ray, tctx.ThreadContext.CameraPath);
	}

	// Handle case where camera ray hits nothing (inf light contribution)
	void handleInfLights(const IterationContext& tctx, CameraTraversalContext& current, const Ray& ray) const
	{
		const uint32 cameraPathLength = ray.IterationDepth + 1;
		tctx.Session.tile()->statistics().addBackgroundHitCount();

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
			const float selProb	 = mLightSampler->pdfLightSelection(light);
			const auto posPDF	 = light->pdfPosition(Vector3f::Zero());
			const float dirPDF_S = light->pdfDirection(ray.Direction);

			float posPDF_A = posPDF.Value;
			if (!posPDF.IsArea) {
				const float r = tctx.Session.context()->scene()->boundingSphere().radius();
				posPDF_A	  = IS::toArea(posPDF_A, r * r, 1);
			}

			const float directPdfA	 = dirPDF_S * selProb; // We lie here
			const float emissivePdfS = posPDF_A * directPdfA;

			// Calculate MIS
			const float cameraMIS = mis_term<Mode>(directPdfA) * current.MIS_VCM + mis_term<Mode>(emissivePdfS) * current.MIS_VC;
			misDenom += cameraMIS;
		}

		// If directly visible from camera, do not calculate mis weights
		if (cameraPathLength == 1) {
			tctx.Session.pushSpectralFragment(1, current.Throughput, radiance, ray, tctx.ThreadContext.CameraPath);
			return;
		}

		// Calculate MIS weights
		const float mis = 1 / (1 + misDenom);

		if (mis <= PR_EPSILON)
			return;

		PR_ASSERT(mis <= 1.0f, "MIS must be between 0 and 1");

		// Splat
		tctx.Session.pushSpectralFragment(mis, current.Throughput, radiance, ray, tctx.ThreadContext.CameraPath);
	}

	inline std::pair<std::vector<PathVertex>::const_iterator, std::vector<PathVertex>::const_iterator>
	lightPath(size_t path) const
	{
		PR_ASSERT(path < mLightPathCounter, "Expected valid path index");
		const auto start = mLightVertices.begin() + path * mLightPathSlice;
		return { start, start + mLightPathSize[path] };
	}

	inline const PathVertex* lightVertex(size_t path, size_t index) const
	{
		PR_ASSERT(path < mLightPathCounter, "Expected valid path index");
		return &mLightVertices[path * mLightPathSlice + index];
	}

private:
	const Options mOptions;
	const Walker mCameraWalker;
	const RussianRoulette mCameraRR;
	const Walker mLightWalker;
	const RussianRoulette mLightRR;
	const std::shared_ptr<LightSampler> mLightSampler;
	const size_t mLightPathSlice;

	std::mutex mMutex;
	std::vector<std::unique_ptr<ThreadContext>> mThreadContexts;

	// Members below are only used when UseMerging=true
	std::atomic<size_t> mLightPathCounter;
	std::vector<PathVertex> mLightVertices;
	std::vector<size_t> mLightPathSize;
	std::vector<size_t> mLightPathWavelengthSortMap;
	std::vector<uint8> mLightPathBuffer; // Contains light path (LPE) in packed form
	std::unique_ptr<PathVertexMap> mLightMap;
};
} // namespace VCM
} // namespace PR