#include "BiDirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "shader/FaceSample.h"
#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"
#include "sampler/MultiJitteredSampler.h"

namespace PR {
BiDirectIntegrator::BiDirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
	, mThreadData(nullptr)
	, mThreadCount(0)
{
}

BiDirectIntegrator::~BiDirectIntegrator()
{
	deleteThreadStructure();
}

void BiDirectIntegrator::deleteThreadStructure()
{
	if (mThreadData) {
		for (uint32 i = 0; i < mThreadCount; ++i) {
			delete[] mThreadData[i].LightVertices;
			delete[] mThreadData[i].LightPathLength;
		}
		delete[] mThreadData;

		mThreadData = nullptr;
	}
}

void BiDirectIntegrator::init()
{
	deleteThreadStructure();

	if (renderer()->lights().empty() || renderer()->settings().maxLightSamples() == 0)
		return;

	mThreadCount		   = renderer()->threads();
	mThreadData			   = new ThreadData[renderer()->threads()];
	size_t maxlightsamples = renderer()->settings().maxRayDepth() * renderer()->lights().size() * renderer()->settings().maxLightSamples();

	for (uint32 i = 0; i < mThreadCount; ++i) {
		mThreadData[i].LightVertices   = new ThreadData::EventVertex[maxlightsamples];
		mThreadData[i].LightPathLength = new uint32[renderer()->lights().size() * renderer()->settings().maxLightSamples()];
	}
}

constexpr float LightEpsilon = 0.00001f;
Spectrum BiDirectIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
{
	if (renderer()->settings().maxLightSamples() == 0)
		return Spectrum();

	PR_ASSERT(mThreadData, "ThreadData not initialized.");

	ShaderClosure other_sc;

	MultiJitteredSampler sampler(context->random(), renderer()->settings().maxLightSamples());
	if (!renderer()->lights().empty()) {
		ThreadData& data = mThreadData[context->threadNumber()];

		const uint32 maxDepth		= renderer()->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer()->settings().maxDiffuseBounces();

		uint32 lightNr = 0;
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				ThreadData::EventVertex* lightV = &data.LightVertices[lightNr * maxDepth];

				float pdf; // Area to Solid Angle?
				other_sc = light->getRandomFacePoint(sampler, i, pdf);

				// Initiate with power
				if (!other_sc.Material->isLight() || pdf <= PR_EPSILON)
					continue;

				Spectrum flux	 = other_sc.Material->emission()->eval(other_sc) / pdf;
				Eigen::Vector3f L = Projection::tangent_align(other_sc.Ng, other_sc.Nx, other_sc.Ny,
															  Projection::cos_hemi(context->random().getFloat(), context->random().getFloat(), pdf));
				float NdotL = std::abs(other_sc.Ng.dot(L));

				uint32 lightDepth = 0; // Counts diff bounces
				lightV[0].Flux	= flux;
				lightV[0].SC	  = other_sc;

				Ray current = Ray::safe(in.pixel(),
										other_sc.P,
										L,
										0,
										in.time(), in.wavelength(),
										in.flags() | RF_Light);

				for (uint32 k = 1;
					 k < maxDepth && lightDepth <= maxDiffBounces && pdf > PR_EPSILON && NdotL > PR_EPSILON;
					 ++k) {
					RenderEntity* entity = context->shoot(current, other_sc);
					if (entity && other_sc.Material && other_sc.Material->canBeShaded()) {
						if (!std::isinf(pdf))
							flux /= MSI::toArea(pdf, other_sc.Depth2, std::abs(other_sc.NdotV));

						L	 = other_sc.Material->sample(other_sc, context->random().get3D(), pdf);
						NdotL = std::abs(other_sc.N.dot(L));

						flux *= other_sc.Material->eval(other_sc, L, NdotL) * NdotL;

						if (!std::isinf(pdf)) {
							lightDepth++;
							lightV[lightDepth].Flux = flux;
							lightV[lightDepth].SC   = other_sc;
						}

						current = current.next(other_sc.P, L);
					} else {
						break;
					}
				}

				data.LightPathLength[lightNr] = lightDepth;
				lightNr++;
			}
		}
	}

	return applyRay(in, context, 0, sc);
}

Spectrum BiDirectIntegrator::applyRay(const Ray& in, RenderThreadContext* context, uint32 diffBounces, ShaderClosure& sc)
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
	Spectrum other_weight;
	float path_weight;
	Spectrum weight;

	RenderEntity* entity = context->shootWithEmission(applied, in, sc);
	if (!entity || !sc.Material || !sc.Material->canBeShaded())
		return applied;

	MultiJitteredSampler sampler(context->random(), maxLightSamples);
	for (uint32 i = 0; i < maxLightSamples && !std::isinf(full_pdf); ++i) {
		float other_pdf			= 0;
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0");
		Eigen::Vector3f rnd = sampler.generate3D(i);
		for (uint32 path = 0; path < path_count && !std::isinf(other_pdf); ++path) {
			float pdf;
			Eigen::Vector3f L = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);
			const float NdotL = std::abs(L.dot(sc.N));

			if (pdf <= PR_EPSILON || NdotL <= PR_EPSILON || !(std::isinf(pdf) || diffBounces < context->renderer()->settings().maxDiffuseBounces()))
				continue;

			weight = applyRay(in.next(sc.P, L),
							  context, !std::isinf(pdf) ? diffBounces + 1 : diffBounces,
							  other_sc);

			weight *= sc.Material->eval(sc, L, NdotL) * NdotL;

			other_pdf += pdf;
			other_weight += path_weight * weight;
		}

		MSI::power(full_weight, full_pdf, other_weight, other_pdf / path_count);
	}

	if (!std::isinf(full_pdf)) {
		const ThreadData& data = mThreadData[context->threadNumber()];

		for (uint32 j = 0; j < maxLights; ++j) // Each light
		{
			for (uint32 s = 0; s < data.LightPathLength[j]; ++s) {
				const ThreadData::EventVertex& lightV = data.LightVertices[j * maxDepth + s];
				const auto LP						  = sc.P - lightV.SC.P;

				Ray current = Ray::safe(in.pixel(),
										lightV.SC.P,
										LP.normalized(),
										in.depth() + 1,
										in.time(), in.wavelength(),
										in.flags() | RF_Light);

				const Eigen::Vector3f L = -current.direction();
				const float NdotL		= std::abs(sc.N.dot(L));

				if (NdotL <= PR_EPSILON)
					continue;

				const float pdf = sc.Material->pdf(sc, L, NdotL);

				if (pdf > PR_EPSILON && context->shoot(current, other_sc) == entity && (sc.P - other_sc.P).squaredNorm() <= LightEpsilon)
					other_weight = lightV.Flux * sc.Material->eval(sc, L, NdotL) * NdotL;
				else
					other_weight.clear();

				MSI::power(full_weight, full_pdf, other_weight, pdf);
			}
		}

		float inf_pdf;
		other_weight = handleInfiniteLights(in, sc, context, inf_pdf);
		MSI::power(full_weight, full_pdf, other_weight, inf_pdf);
	}

	return applied + full_weight;
}
}
