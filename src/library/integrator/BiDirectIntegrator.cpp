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
	const Eigen::Vector3f rnd = tile->random().get3D();
	if (!renderer()->lights().empty()) {
		TileData& data = mTileData[tile->index()];

		const uint32 maxDepth		= renderer()->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer()->settings().maxDiffuseBounces();

		uint32 lightNr = 0;
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				TileData::EventVertex* lightV = &data.LightVertices[lightNr * maxDepth];

				RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd, i);

				// Initiate with power
				if (!fps.Point.Material->isLight() || fps.PDF <= PR_EPSILON)
					continue;

				Spectrum flux = fps.Point.Material->emission()->eval(fps.Point) / fps.PDF;

				MaterialSample ms;
				ms.L = Projection::tangent_align(fps.Point.Ng, fps.Point.Nx, fps.Point.Ny,
												 Projection::cos_hemi(tile->random().getFloat(), tile->random().getFloat(), ms.PDF));
				float NdotL = std::abs(fps.Point.Ng.dot(ms.L));

				uint32 lightDepth = 0; // Counts diff bounces
				lightV[0].Flux	= flux;
				lightV[0].SC	  = fps.Point;
				lightV[0].PDF	 = ms.PDF;

				Ray current = Ray::safe(in.pixel(),
										other_sc.P,
										ms.L,
										0,
										in.time(), in.wavelength(),
										in.flags() | RF_Light);

				for (uint32 k = 1;
					 k < maxDepth && lightDepth <= maxDiffBounces && ms.PDF > PR_EPSILON && NdotL > PR_EPSILON;
					 ++k) {
					RenderEntity* entity = renderer()->shoot(current, other_sc, tile);
					if (entity && other_sc.Material && other_sc.Material->canBeShaded()) {
						ms	= other_sc.Material->sample(other_sc, tile->random().get3D());
						NdotL = std::abs(other_sc.N.dot(ms.L));

						flux *= other_sc.Material->eval(other_sc, ms.L, NdotL) * NdotL;

						if (!std::isinf(ms.PDF)) {
							lightDepth++;
							lightV[lightDepth].Flux = flux;
							lightV[lightDepth].SC   = other_sc;
							lightV[lightDepth].PDF  = lightV[lightDepth - 1].PDF * ms.PDF;
							PR_ASSERT(!std::isinf(lightV[lightDepth].PDF), "No light vertex should be specular");
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

	if (in.depth() >= maxDepth)
		return Spectrum();

	Spectrum applied;
	Spectrum full_weight;
	float full_pdf = 0;

	// Temporary
	ShaderClosure other_sc;
	Spectrum weight;

	RenderEntity* entity = renderer()->shootWithEmission(applied, in, sc, tile);
	if (!entity || !sc.Material || !sc.Material->canBeShaded())
		return applied;

	MultiJitteredSampler sampler(tile->random(), maxLightSamples);
	for (uint32 i = 0; i < maxLightSamples && !std::isinf(full_pdf); ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0");
		Eigen::Vector3f rnd = sampler.generate3D(i);
		for (uint32 path = 0; path < path_count && !std::isinf(full_pdf); ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF <= PR_EPSILON || NdotL <= PR_EPSILON || !(std::isinf(ms.PDF) || diffBounces < renderer()->settings().maxDiffuseBounces()))
				continue;

			weight = applyRay(in.next(sc.P, ms.L),
							  tile, !std::isinf(ms.PDF) ? diffBounces + 1 : diffBounces,
							  other_sc);

			weight *= sc.Material->eval(sc, ms.L, NdotL) * NdotL;
			MSI::balance(full_weight, full_pdf, ms.Weight * weight, ms.PDF);
		}
	}

	if (!std::isinf(full_pdf)) {
		const TileData& data = mTileData[tile->index()];

		// Each light
		for (uint32 j = 0; j < maxLights; ++j) {
			for (uint32 s = 0; s < data.LightPathLength[j]; ++s) {
				const TileData::EventVertex& lightV = data.LightVertices[j * maxDepth + s];
				const auto LP						= sc.P - lightV.SC.P;

				Ray current = Ray::safe(in.pixel(),
										lightV.SC.P,
										LP.normalized(),
										in.depth(),
										in.time(), in.wavelength(),
										in.flags() | RF_Light);

				const Eigen::Vector3f L = -current.direction();
				const float NdotL		= std::abs(sc.N.dot(L));

				if (NdotL <= PR_EPSILON)
					continue;

				const float pdf = sc.Material->pdf(sc, L, NdotL) * lightV.PDF;

				if (pdf > PR_EPSILON && renderer()->shoot(current, other_sc, tile) == entity && (sc.P - other_sc.P).squaredNorm() <= LightEpsilon)
					weight = lightV.Flux * sc.Material->eval(sc, L, NdotL) * NdotL;
				else
					weight.clear();

				MSI::balance(full_weight, full_pdf, weight, pdf);
			}
		}

		float inf_pdf;
		weight = handleInfiniteLights(in, sc, tile, inf_pdf);
		MSI::balance(full_weight, full_pdf, weight, inf_pdf);
	}

	return applied + full_weight;
}
}
