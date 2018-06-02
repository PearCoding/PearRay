#include "BiDirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"
#include "sampler/MultiJitteredSampler.h"

#include <vector>

namespace PR {

static const char* RE_LIGHT_SAMPLES = "bidirect/light/sample_count";
static const char* RE_LIGHT_DEPTH   = "bidirect/light/max_depth";
static const char* RE_DIFFUSE_DEPTH = "bidirect/diffuse/max_depth";

struct BIDI_TileData {
	struct EventVertex {
		Spectrum Flux;
		ShaderClosure SC;
		float PDF;
		RenderEntity* Entity;

		inline EventVertex(const Spectrum& spec, const ShaderClosure& sc, float pdf, RenderEntity* e)
			: Flux(spec)
			, SC(sc)
			, PDF(pdf)
			, Entity(e)
		{
		}
	};

	std::vector<EventVertex> LightVertices;
	std::vector<uint32> LightPathLength;
	//EventVertex* EyeVertices;
};

struct BIDI_ThreadData {
	std::vector<Spectrum> PathWeights;
	std::vector<Spectrum> Weights;
	std::vector<Spectrum> Evaluations;
	std::vector<Spectrum> HemiWeights;
	Spectrum IntegralWeight;
	Spectrum LiWeight;

	explicit BIDI_ThreadData(RenderContext* context)
		: IntegralWeight(context->spectrumDescriptor())
		, LiWeight(context->spectrumDescriptor())
	{
		for (uint32 i = 0; i < context->settings().maxRayDepth(); ++i) {
			PathWeights.emplace_back(context->spectrumDescriptor());
			Weights.emplace_back(context->spectrumDescriptor());
			Evaluations.emplace_back(context->spectrumDescriptor());
			HemiWeights.emplace_back(context->spectrumDescriptor());
		}
	}
};

BiDirectIntegrator::BiDirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

BiDirectIntegrator::~BiDirectIntegrator()
{
}

void BiDirectIntegrator::init()
{
	OnePassIntegrator::init();

	mTileData.clear();
	mThreadData.clear();

	if (renderer()->lights().empty()) {
		throw std::runtime_error("Bad integrator environment");
	}

	mMaxLightSamples = std::max<uint32>(1,
										renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				   RE_LIGHT_SAMPLES,
																				   1));

	mMaxLightDepth  = std::max<uint32>(1,
									   renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				  RE_LIGHT_DEPTH,
																				  1));
	mMaxDiffBounces = renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																 RE_DIFFUSE_DEPTH,
																 1);

	mMaxCameraDepth  = renderer()->settings().maxRayDepth();
	mMaxLightPatches = renderer()->lights().size() * mMaxLightSamples;
	for (uint32 i = 0; i < renderer()->tileCount(); ++i)
		mTileData.emplace_back();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());

	size_t fulllightsamples = mMaxLightDepth * mMaxLightPatches;
	for (BIDI_TileData& tdata : mTileData) {
		tdata.LightVertices   = std::vector<BIDI_TileData::EventVertex>(fulllightsamples, BIDI_TileData::EventVertex(Spectrum(renderer()->spectrumDescriptor()), ShaderClosure(), 0.0f, nullptr));
		tdata.LightPathLength = std::vector<uint32>(mMaxLightPatches, 0);
	}
}

constexpr float LightEpsilon = 0.00001f;
constexpr float SpecEpsilon  = 0.0001f;
void BiDirectIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	ShaderClosure other_sc;

	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	BIDI_TileData& data			= mTileData[session.tile()->index()];

	uint32 lightNr = 0;
	for (RenderEntity* light : renderer()->lights()) {
		BIDI_TileData::EventVertex* lightV = &data.LightVertices[lightNr * mMaxLightDepth];

		for (uint32 i = 0; i < mMaxLightSamples; ++i) {
			const Eigen::Vector3f rnd		  = session.tile()->random().get3D();
			RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd);

			// Skip light if needed
			if (!fps.Point.Material->isLight() || fps.PDF_A <= PR_EPSILON) {
				lightV[0].PDF				  = 0;
				lightV[0].Entity			  = nullptr;
				data.LightPathLength[lightNr] = 0;
				lightNr++;
				continue;
			}

			// Initiate with power
			fps.Point.Material->evalEmission(threadData.Weights[0], fps.Point, session);
			//threadData.Weights[0] *= light->surfaceArea(fps.Point.Material);

			if (threadData.Weights[0].isOnlyZero(SpecEpsilon)) {
				lightV[0].PDF				  = 0;
				lightV[0].Entity			  = nullptr;
				data.LightPathLength[lightNr] = 0;
				lightNr++;
				continue;
			}

			uint32 lightDepth = 0;
			// Initial entry -> Light
			lightV[0].Flux.copyFrom(threadData.Weights[0]);
			lightV[0].SC	 = fps.Point;
			lightV[0].PDF	= fps.PDF_A; // The direct light entry has an area pdf
			lightV[0].Entity = light;
			lightDepth++;

			threadData.Weights[0] /= fps.PDF_A;

			MaterialSample ms;
			ms.L		= Projection::tangent_align(fps.Point.Ng, fps.Point.Nx, fps.Point.Ny,
												Projection::cos_hemi(session.tile()->random().getFloat(), session.tile()->random().getFloat(), ms.PDF_S));
			float NdotL = std::abs(fps.Point.Ng.dot(ms.L));
			Ray current = Ray(in.pixel(),
							  Ray::safePosition(other_sc.P, ms.L),
							  ms.L,
							  0,
							  in.time(), in.wavelength(),
							  in.flags() | RF_Light);

			float lastPdfS = ms.PDF_S;
			for (uint32 k = 1;
				 k < mMaxLightDepth && lightDepth <= mMaxDiffBounces
				 && ms.PDF_S > PR_EPSILON && NdotL > PR_EPSILON;
				 ++k) {
				RenderEntity* entity = renderer()->shoot(current, other_sc, session);
				if (entity && other_sc.Material && other_sc.Material->canBeShaded()) {
					ms = other_sc.Material->sample(
						other_sc, session.tile()->random().get3D(), session);
					NdotL = std::abs(other_sc.N.dot(ms.L));

					other_sc.Material->eval(threadData.Evaluations[0], other_sc, ms.L, NdotL, session);
					threadData.Weights[0] *= threadData.Evaluations[0] * NdotL;

					if (!ms.isSpecular()) {
						lightV[lightDepth].Flux.copyFrom(threadData.Weights[0]);
						lightV[lightDepth].SC	 = other_sc;
						lightV[lightDepth].PDF	= lastPdfS;
						lightV[lightDepth].Entity = entity;
						PR_ASSERT(!std::isinf(lightV[lightDepth].PDF), "No light vertex should be specular");

						lightDepth++;
						threadData.Weights[0] /= ms.PDF_S;
						lastPdfS = ms.PDF_S;
					}

					current = current.next(other_sc.P, ms.L);
				} else {
					break;
				}
			}

			data.LightPathLength[lightNr] = lightDepth;
			lightNr++;
		}
	}

	applyRay(spec, in, session, 0, sc);
}

/**
 * Bidirectional Raytracing
 * 3 Sampling Techniques
 *   1. Hemisphere Sampling (Distributed Sampling)
 *   2. Light Connection
 *   3. Infinite Sampling (Environment)
 */
void BiDirectIntegrator::applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffBounces, ShaderClosure& sc)
{
	RenderEntity* entity = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded())
		return;

	//const bool specular = addIndirectContribution(spec, in, session, diffBounces, sc);

	//if (!specular) {
	addDirectContribution(spec, in, session, sc);
	//addInfiniteLightContribution(spec, in, sc, session, mMaxLightSamples);
	//}
}

void BiDirectIntegrator::addDirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	const BIDI_TileData& data   = mTileData[session.tile()->index()];

	threadData.IntegralWeight.clear();

	// For each light patch
	size_t counter = 0;
	for (uint32 j = 0; j < mMaxLightPatches; ++j) {
		if (data.LightPathLength[j] == 0)
			continue;

		const Eigen::Vector3f rnd = session.tile()->random().get3D();
		sampleLightPatch(threadData.LiWeight, j, rnd, in, session, sc);

		threadData.IntegralWeight += threadData.LiWeight;
		++counter;
	}

	if (counter > 0)
		spec += threadData.IntegralWeight / (float)counter;
}

bool BiDirectIntegrator::addIndirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth			= in.depth();

	const Eigen::Vector3f rnd_hemi = session.tile()->random().get3D();
	const bool specular			   = sampleHemisphere(threadData.HemiWeights[depth],
											  rnd_hemi, in, session, diffbounces, sc);
	spec += threadData.HemiWeights[depth];

	return specular;
}

bool BiDirectIntegrator::sampleHemisphere(Spectrum& spec, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc)
{
	spec.clear();

	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth			= in.depth();
	const uint32 path_count		= sc.Material->samplePathCount();

	bool specular = false;
	for (uint32 path = 0; path < path_count; ++path) {
		const MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);

		float NdotL = 0;
		if (ms.isReflection())
			NdotL = std::max(0.0f, ms.L.dot(sc.N));
		else
			NdotL = std::max(0.0f, -ms.L.dot(sc.N));

		specular = specular || ms.isSpecular();

		const float W				 = ms.PathWeight * NdotL;
		const uint32 nextDiffBounces = !ms.isSpecular() ? diffbounces + 1 : diffbounces;
		if (ms.PDF_S <= PR_EPSILON || W <= PR_EPSILON
			|| nextDiffBounces > mMaxDiffBounces) {
			continue;
		}

		sc.Material->eval(threadData.Evaluations[depth], sc, ms.L, NdotL, session);
		threadData.Evaluations[depth] *= W / ms.safePDF_S();
		if (threadData.Evaluations[depth].isOnlyZero(SpecEpsilon)) {
			// TODO: Careful -> What with high radiance?
			continue;
		}

		ShaderClosure other_sc;
		applyRay(threadData.PathWeights[depth], in.next(sc.P, ms.L), session,
				 nextDiffBounces, other_sc);

		spec += threadData.PathWeights[depth] * threadData.Evaluations[depth];
	}

	return specular;
}

void BiDirectIntegrator::sampleLightPatch(Spectrum& spec, uint32 patch, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	spec.clear();

	BIDI_TileData& data = mTileData[session.tile()->index()];

	float full_pdf = 0;

	// Special case s = 0 -> Direct Light
	sampleLightPatchFirstNode(spec, full_pdf, patch * mMaxLightDepth, rnd, in, session, sc);

	/* We have to calculate the flux and pdf entry of the "light vertex" and of the connection */
	for (uint32 s = 1; s < data.LightPathLength[patch]; ++s) {
		sampleLightPatchOtherNodes(spec, full_pdf, patch * mMaxLightDepth + s, rnd, in, session, sc);
	}
}

void BiDirectIntegrator::sampleLightPatchFirstNode(Spectrum& spec, float& full_pdf, uint32 index, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	ShaderClosure other_sc;
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	BIDI_TileData& data			= mTileData[session.tile()->index()];
	const uint32 depth			= in.depth();

	const auto& lightV = data.LightVertices[index];
	if (lightV.Entity == nullptr)
		return;

	const auto PS		   = lightV.SC.P - sc.P;
	const auto L		   = PS.normalized();
	const float NdotL	  = sc.N.dot(L);
	const float lightNdotV = std::abs(L.dot(lightV.SC.Ng));

	const float geom = std::abs(NdotL * lightNdotV);

	if (geom <= PR_EPSILON)
		return;

	sc.Material->eval(threadData.Evaluations[depth], sc, L, NdotL, session);
	threadData.Evaluations[depth] *= geom;

	if (threadData.Evaluations[depth].isOnlyZero(SpecEpsilon))
		return;

	// The direct light entry is given in pdf respect to surface area
	const float pdfS = MSI::toSolidAngle(lightV.PDF,
										 PS.squaredNorm(), geom);

	Ray ray = in.next(sc.P, L);
	ray.setFlags(ray.flags() | RF_Light);

	if (renderer()->shoot(ray, other_sc, session) == lightV.Entity
		&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon) {

		if (other_sc.Flags & SCF_Inside) // If wrong side (Back side)
			return;

		MSI::balance(spec, full_pdf,
					 lightV.Flux * threadData.Evaluations[depth] /* lightV.Entity->surfaceArea(other_sc.Material)*/,
					 pdfS);
	}
}

void BiDirectIntegrator::sampleLightPatchOtherNodes(Spectrum& spec, float& full_pdf, uint32 index, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	ShaderClosure other_sc;
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	BIDI_TileData& data			= mTileData[session.tile()->index()];
	const uint32 depth			= in.depth();

	const auto& lightV	 = data.LightVertices[index];
	const auto& lastLightV = data.LightVertices[index - 1];
	const auto PS		   = lightV.SC.P - sc.P;
	const auto L		   = PS.normalized();

	const float NdotL = /*std::max(0.0f, */ sc.N.dot(L) /*)*/;

	if (NdotL <= PR_EPSILON)
		return;

	{ // Recalculate light vertex flux
		other_sc   = lightV.SC;
		other_sc.V = L;
		// TODO: dVdX, dVdY
		const float pdfS	  = lightV.PDF;
		const float lastNdotL = std::abs(other_sc.N.dot(lastLightV.SC.V));
		lightV.SC.Material->eval(threadData.Evaluations[depth], other_sc, lastLightV.SC.V, lastNdotL, session);
		threadData.PathWeights[depth] = lastLightV.Flux * threadData.Evaluations[depth] * lastNdotL;
		threadData.PathWeights[depth] /= pdfS;
	}

	if (threadData.PathWeights[depth].isOnlyZero(SpecEpsilon))
		return;

	{ // Calculate connection
		Ray ray = in.next(sc.P, L);
		ray.setFlags(ray.flags() | RF_Light);

		sc.Material->eval(threadData.Evaluations[depth], sc, L, NdotL, session);
		threadData.Evaluations[depth] *= NdotL;
		if (threadData.Evaluations[depth].isOnlyZero(SpecEpsilon))
			return;

		if (renderer()->shootWithEmission(threadData.Weights[depth], ray, other_sc, session) == lightV.Entity /*&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
			threadData.Weights[depth] += threadData.PathWeights[depth] * threadData.Evaluations[depth];
			MSI::balance(spec, full_pdf, threadData.Weights[depth], PR_1_PI); // FIXME: Really fixed 1/pi?
		}
	}
}
} // namespace PR
