#include "BiDirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"
#include "sampler/MultiJitteredSampler.h"

namespace PR {
BiDirectIntegrator::BiDirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
	, mTileData(nullptr)
	, mTileCount(0)
{
}

BiDirectIntegrator::~BiDirectIntegrator()
{
	deleteTileStructure();
}

void BiDirectIntegrator::deleteTileStructure()
{
	if (mTileData) {
		for (uint32 i = 0; i < mTileCount; ++i) {
			delete[] mTileData[i].LightVertices;
			delete[] mTileData[i].LightPathLength;
		}
		delete[] mTileData;

		mTileData = nullptr;
	}
}

void BiDirectIntegrator::init()
{
	deleteTileStructure();

	if (renderer()->lights().empty() || renderer()->settings().maxLightSamples() == 0)
		return;

	mTileCount			   = renderer()->tileCount();
	mTileData			   = new TileData[mTileCount];
	size_t maxlightsamples = renderer()->settings().maxRayDepth() * renderer()->lights().size() * renderer()->settings().maxLightSamples();

	for (uint32 i = 0; i < mTileCount; ++i) {
		mTileData[i].LightVertices   = new TileData::EventVertex[maxlightsamples];
		mTileData[i].LightPathLength = new uint32[renderer()->lights().size() * renderer()->settings().maxLightSamples()];
	}
}

constexpr float LightEpsilon = 0.00001f;
Spectrum BiDirectIntegrator::apply(const Ray& in, RenderTile* tile, uint32 pass, ShaderClosure& sc)
{
	if (renderer()->settings().maxLightSamples() == 0)
		return Spectrum();

	PR_ASSERT(mTileData, "TileData not initialized.");

	ShaderClosure other_sc;

	//MultiJitteredSampler sampler(tile->random(), renderer()->settings().maxLightSamples());
	if (!renderer()->lights().empty()) {
		TileData& data = mTileData[tile->index()];

		const uint32 maxDepth		= renderer()->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer()->settings().maxDiffuseBounces();

		uint32 lightNr = 0;
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				TileData::EventVertex* lightV = &data.LightVertices[lightNr * maxDepth];

				const Eigen::Vector3f rnd = tile->random().get3D();
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
				Spectrum flux = fps.Point.Material->emission()->eval(fps.Point);
				flux *= light->surfaceArea(fps.Point.Material);

				uint32 lightDepth = 0;
				// Initial entry -> Light
				lightV[0].Flux   = flux;
				lightV[0].SC	 = fps.Point;
				lightV[0].PDF	= fps.PDF_A; // The direct light entry has an area pdf
				lightV[0].Entity = light;
				lightDepth++;

				flux /= fps.PDF_A;

				MaterialSample ms;
				ms.L = Projection::tangent_align(fps.Point.Ng, fps.Point.Nx, fps.Point.Ny,
												 Projection::cos_hemi(tile->random().getFloat(), tile->random().getFloat(), ms.PDF_S));
				float NdotL = std::abs(fps.Point.Ng.dot(ms.L));
				Ray current = Ray::safe(in.pixel(),
										other_sc.P,
										ms.L,
										0,
										in.time(), in.wavelength(),
										in.flags() | RF_Light);

				float lastPdfS = ms.PDF_S;
				for (uint32 k = 1;
					 k < maxDepth && lightDepth <= maxDiffBounces && ms.PDF_S > PR_EPSILON && NdotL > PR_EPSILON;
					 ++k) {
					const Eigen::Vector3f lastP = other_sc.P;

					RenderEntity* entity = renderer()->shoot(current, other_sc, tile);
					if (entity && other_sc.Material && other_sc.Material->canBeShaded()) {
						ms	= other_sc.Material->sample(other_sc, tile->random().get3D());
						NdotL = std::abs(other_sc.N.dot(ms.L));

						flux *= other_sc.Material->eval(other_sc, ms.L, NdotL) * NdotL;

						if (!ms.isSpecular()) {
							lightV[lightDepth].Flux   = flux;
							lightV[lightDepth].SC	 = other_sc;
							lightV[lightDepth].PDF	= lastPdfS;
							lightV[lightDepth].Entity = entity;
							PR_ASSERT(!std::isinf(lightV[lightDepth].PDF), "No light vertex should be specular");

							lightDepth++;
							flux /= ms.PDF_S;
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

	return applyRay(in, tile, 0, sc);
}

Spectrum BiDirectIntegrator::applyRay(const Ray& in, RenderTile* tile, uint32 diffBounces, ShaderClosure& sc)
{
	const uint32 maxLightSamples = renderer()->settings().maxLightSamples();
	const uint32 maxLights		 = maxLightSamples * (uint32)renderer()->lights().size();
	const uint32 maxDepth		 = renderer()->settings().maxRayDepth();

	Spectrum applied;
	RenderEntity* entity = renderer()->shootWithEmission(applied, in, sc, tile);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffBounces > renderer()->settings().maxDiffuseBounces())
		return applied;

	Spectrum full_weight;
	float full_pdf = 0;

	// Temporary
	ShaderClosure other_sc;
	Spectrum weight;

	bool noSpecular = true;

	MultiJitteredSampler sampler(tile->random(), maxLightSamples);
	for (uint32 i = 0; i < maxLightSamples && noSpecular; ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0");
		Eigen::Vector3f rnd = sampler.generate3D(i);
		for (uint32 path = 0; path < path_count && noSpecular; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF_S <= PR_EPSILON || NdotL <= PR_EPSILON)
				continue;

			weight = applyRay(in.next(sc.P, ms.L), tile,
							  !std::isinf(ms.PDF_S) ? diffBounces + 1 : diffBounces,
							  other_sc);

			//if (!weight.isOnlyZero()) {
			weight *= sc.Material->eval(sc, ms.L, NdotL) * NdotL;
			MSI::balance(full_weight, full_pdf, weight, ms.PDF_S);
			//}

			if (ms.isSpecular())
				noSpecular = false;
		}
	}

	if (noSpecular) {
		Spectrum inFlux;
		const TileData& data = mTileData[tile->index()];

		// Each light
		for (uint32 j = 0; j < maxLights; ++j) {
			if (data.LightPathLength[j] == 0)
				continue;

			// Special case s = 0 -> Direct Light
			{
				const TileData::EventVertex& lightV = data.LightVertices[j * maxDepth];
				const auto PS						= lightV.SC.P - sc.P;
				const auto L						= PS.normalized();
				const float NdotL					= std::abs(sc.N.dot(L));
				const float lightNdotV				= std::abs(L.dot(lightV.SC.Ng));

				// The direct light entry is given in pdf respect to surface area
				const float pdfS = MSI::toSolidAngle(lightV.PDF,
													 PS.squaredNorm(), std::abs(NdotL * sc.NdotV));

				Ray ray = in.next(sc.P, L);
				ray.setFlags(ray.flags() | RF_Light);

				if (renderer()->shoot(ray, other_sc, tile) == lightV.Entity /*&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
					if (other_sc.Flags & SCF_Inside) // Wrong side (Back side)
						continue;

					weight = lightV.Flux * sc.Material->eval(sc, L, NdotL) * NdotL * lightNdotV;
					MSI::balance(full_weight, full_pdf, weight, pdfS);
				}
			}

			/* We have to calculate the flux and pdf entry of the "light vertex" and of the connection */
			for (uint32 s = 1; s < data.LightPathLength[j]; ++s) {
				const TileData::EventVertex& lightV		= data.LightVertices[j * maxDepth + s];
				const TileData::EventVertex& lastLightV = data.LightVertices[j * maxDepth + s - 1];
				const auto PS							= lightV.SC.P - sc.P;
				const auto L							= PS.normalized();

				const float NdotL = std::abs(sc.N.dot(L));

				if (NdotL <= PR_EPSILON)
					continue;

				{ // Recalculate light vertex flux
					other_sc   = lightV.SC;
					other_sc.V = L;
					// TODO: dVdX, dVdY
					const float pdfS	  = lightV.PDF;
					const float lastNdotL = std::abs(other_sc.N.dot(lastLightV.SC.V));
					inFlux				  = lastLightV.Flux * lightV.SC.Material->eval(other_sc, lastLightV.SC.V, lastNdotL) * lastNdotL;
					inFlux /= pdfS;
				}

				{ // Calculate connection
					Ray ray = in.next(sc.P, L);
					ray.setFlags(ray.flags() | RF_Light);

					if (renderer()->shootWithEmission(weight, ray, other_sc, tile) == lightV.Entity /*&& (sc.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
						weight += inFlux * sc.Material->eval(sc, L, NdotL) * NdotL;
						MSI::balance(full_weight, full_pdf, weight, PR_1_PI); // FIXME: Really fixed 1/pi?
					}
				}
			}
		}

		float inf_pdf;
		weight = handleInfiniteLights(in, sc, tile, inf_pdf);
		MSI::balance(full_weight, full_pdf, weight, inf_pdf);
	}

	return applied + full_weight;
}
}
