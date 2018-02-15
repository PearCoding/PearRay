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
	bool Lock;
	std::vector<Spectrum> FullWeight;
	std::vector<Spectrum> Weight;
	std::vector<Spectrum> Evaluation;

	explicit BIDI_ThreadData(RenderContext* context)
		: Lock(false)
		, FullWeight(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Weight(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Evaluation(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
	{
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

	if (renderer()->lights().empty() || renderer()->settings().maxLightSamples() == 0)
		return;

	for (uint32 i = 0; i < renderer()->tileCount(); ++i)
		mTileData.emplace_back();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());

	size_t maxlightsamples = renderer()->settings().maxRayDepth() * renderer()->lights().size() * renderer()->settings().maxLightSamples();

	for (BIDI_TileData& tdata : mTileData) {
		tdata.LightVertices   = std::vector<BIDI_TileData::EventVertex>(maxlightsamples, BIDI_TileData::EventVertex(Spectrum(renderer()->spectrumDescriptor()), ShaderClosure(), 0.0f, nullptr));
		tdata.LightPathLength = std::vector<uint32>(renderer()->lights().size() * renderer()->settings().maxLightSamples(), 0);
	}
}

constexpr float LightEpsilon = 0.00001f;
void BiDirectIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	if (renderer()->settings().maxLightSamples() == 0)
		return;

	ShaderClosure other_sc;

	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	PR_ASSERT(threadData.Lock == false, "No recursive call of Integrators allowed");
	threadData.Lock = true; // Only for development purposes

	//MultiJitteredSampler sampler(tile->random(), renderer()->settings().maxLightSamples());
	if (!renderer()->lights().empty()) {
		BIDI_TileData& data = mTileData[session.tile()->index()];

		const uint32 maxDepth		= renderer()->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer()->settings().maxDiffuseBounces();

		uint32 lightNr = 0;
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				BIDI_TileData::EventVertex* lightV = &data.LightVertices[lightNr * maxDepth];

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
				fps.Point.Material->evalEmission(threadData.Weight[0], fps.Point, session);
				threadData.Weight[0] *= light->surfaceArea(fps.Point.Material);

				uint32 lightDepth = 0;
				// Initial entry -> Light
				lightV[0].Flux   = threadData.Weight[0];
				lightV[0].SC	 = fps.Point;
				lightV[0].PDF	= fps.PDF_A; // The direct light entry has an area pdf
				lightV[0].Entity = light;
				lightDepth++;

				threadData.Weight[0] /= fps.PDF_A;

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
					 k < maxDepth && lightDepth <= maxDiffBounces && ms.PDF_S > PR_EPSILON && NdotL > PR_EPSILON;
					 ++k) {
					RenderEntity* entity = renderer()->shoot(current, other_sc, session);
					if (entity && other_sc.Material && other_sc.Material->canBeShaded()) {
						ms = other_sc.Material->sample(
							other_sc, session.tile()->random().get3D(), session);
						NdotL = std::abs(other_sc.N.dot(ms.L));

						other_sc.Material->eval(threadData.Evaluation[0], other_sc, ms.L, NdotL, session);
						threadData.Weight[0] *= threadData.Evaluation[0] * NdotL;

						if (!ms.isSpecular()) {
							lightV[lightDepth].Flux   = threadData.Weight[0];
							lightV[lightDepth].SC	 = other_sc;
							lightV[lightDepth].PDF	= lastPdfS;
							lightV[lightDepth].Entity = entity;
							PR_ASSERT(!std::isinf(lightV[lightDepth].PDF), "No light vertex should be specular");

							lightDepth++;
							threadData.Weight[0] /= ms.PDF_S;
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
	}

	applyRay(spec, in, session, 0, sc);

	threadData.Lock = false;
}

void BiDirectIntegrator::applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffBounces, ShaderClosure& sc)
{
	const uint32 maxLightSamples = renderer()->settings().maxLightSamples();
	const uint32 maxLights		 = maxLightSamples * renderer()->lights().size();
	const uint32 maxDepth		 = renderer()->settings().maxRayDepth();

	RenderEntity* entity = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffBounces > renderer()->settings().maxDiffuseBounces())
		return;

	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth			= in.depth();

	float full_pdf = 0;

	// Temporary
	ShaderClosure other_sc;

	bool noSpecular = true;

	MultiJitteredSampler sampler(session.tile()->random(), maxLightSamples);
	for (uint32 i = 0; i < maxLightSamples && noSpecular; ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0");
		Eigen::Vector3f rnd = sampler.generate3D(i);
		for (uint32 path = 0; path < path_count && noSpecular; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF_S <= PR_EPSILON || NdotL <= PR_EPSILON)
				continue;

			// TODO: Serialize
			applyRay(threadData.Weight[depth], in.next(sc.P, ms.L), session,
					 !std::isinf(ms.PDF_S) ? diffBounces + 1 : diffBounces,
					 other_sc);

			sc.Material->eval(threadData.Evaluation[depth], sc, ms.L, NdotL, session);
			threadData.Weight[depth] *= threadData.Evaluation[depth] * NdotL;
			MSI::balance(spec, full_pdf, threadData.Weight[depth], ms.PDF_S);

			if (ms.isSpecular())
				noSpecular = false;
		}
	}

	if (noSpecular) {
		const BIDI_TileData& data = mTileData[session.tile()->index()];

		// Each light
		for (uint32 j = 0; j < maxLights; ++j) {
			if (data.LightPathLength[j] == 0)
				continue;

			// Special case s = 0 -> Direct Light
			{
				const BIDI_TileData::EventVertex& lightV = data.LightVertices[j * maxDepth];
				const auto PS							 = lightV.SC.P - sc.P;
				const auto L							 = PS.normalized();
				const float NdotL						 = std::abs(sc.N.dot(L));
				const float lightNdotV					 = std::abs(L.dot(lightV.SC.Ng));

				// The direct light entry is given in pdf respect to surface area
				const float pdfS = MSI::toSolidAngle(lightV.PDF,
													 PS.squaredNorm(), std::abs(NdotL * sc.NdotV));

				Ray ray = in.next(sc.P, L);
				ray.setFlags(ray.flags() | RF_Light);

				if (renderer()->shoot(ray, other_sc, session) == lightV.Entity /*&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
					if (other_sc.Flags & SCF_Inside) // Wrong side (Back side)
						continue;

					sc.Material->eval(threadData.Evaluation[depth], sc, L, NdotL, session);
					threadData.Weight[depth] = lightV.Flux * threadData.Evaluation[depth] * NdotL * lightNdotV;
					MSI::balance(spec, full_pdf, threadData.Weight[depth], pdfS);
				}
			}

			/* We have to calculate the flux and pdf entry of the "light vertex" and of the connection */
			for (uint32 s = 1; s < data.LightPathLength[j]; ++s) {
				const BIDI_TileData::EventVertex& lightV	 = data.LightVertices[j * maxDepth + s];
				const BIDI_TileData::EventVertex& lastLightV = data.LightVertices[j * maxDepth + s - 1];
				const auto PS								 = lightV.SC.P - sc.P;
				const auto L								 = PS.normalized();

				const float NdotL = std::abs(sc.N.dot(L));

				if (NdotL <= PR_EPSILON)
					continue;

				{ // Recalculate light vertex flux
					other_sc   = lightV.SC;
					other_sc.V = L;
					// TODO: dVdX, dVdY
					const float pdfS	  = lightV.PDF;
					const float lastNdotL = std::abs(other_sc.N.dot(lastLightV.SC.V));
					lightV.SC.Material->eval(threadData.Evaluation[depth], other_sc, lastLightV.SC.V, lastNdotL, session);
					threadData.FullWeight[depth] = lastLightV.Flux * threadData.Evaluation[depth] * lastNdotL;
					threadData.FullWeight[depth] /= pdfS;
				}

				{ // Calculate connection
					Ray ray = in.next(sc.P, L);
					ray.setFlags(ray.flags() | RF_Light);

					if (renderer()->shootWithEmission(threadData.Weight[depth], ray, other_sc, session) == lightV.Entity /*&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
						sc.Material->eval(threadData.Evaluation[depth], sc, L, NdotL, session);
						threadData.Weight[depth] += threadData.FullWeight[depth] * threadData.Evaluation[depth] * NdotL;
						MSI::balance(spec, full_pdf, threadData.Weight[depth], PR_1_PI); // FIXME: Really fixed 1/pi?
					}
				}
			}
		}

		float inf_pdf;
		handleInfiniteLights(threadData.Evaluation[depth], in, sc, session, inf_pdf);
		MSI::balance(spec, full_pdf, threadData.Evaluation[depth], inf_pdf);
	}
}
} // namespace PR
