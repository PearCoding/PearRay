#include "Integrator.h"

#include "light/IInfiniteLight.h"
#include "spectral/Spectrum.h"

#include "sampler/RandomSampler.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"

#include "scene/Scene.h"

#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "ray/Ray.h"

#include "math/MSI.h"

namespace PR {
Spectrum Integrator::handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderTile* tile, float& full_pdf)
{
	Spectrum full_weight;
	full_pdf = 0;

	if(renderer()->scene().infiniteLights().empty())
		return full_weight;

	const float lightSampleWeight = 1.0f/renderer()->settings().maxLightSamples();
	const float inflightCountWeight = 1.0f/renderer()->scene().infiniteLights().size();

	RandomSampler sampler(tile->random());
	for (const auto& e : renderer()->scene().infiniteLights()) {
		Spectrum semi_weight;
		float semi_pdf = 0;

		for (uint32 i = 0;
			 i < renderer()->settings().maxLightSamples() && !std::isinf(semi_pdf);
			 ++i) {
			IInfiniteLight::LightSample ls = e->sample(sc, sampler.generate3D(i));

			if (ls.PDF_S <= PR_EPSILON)
				continue;

			Spectrum weight;
			const float NdotL = std::abs(ls.L.dot(sc.N));

			if(NdotL <= PR_EPSILON)
				continue;

			RenderEntity* entity;

			Ray ray = in.next(sc.P, ls.L);
			ray.setFlags(ray.flags() | RF_Light);

			weight = handleSpecularPath(ray, sc, tile, entity);
			if (!entity)
				weight *= sc.Material->eval(sc, ls.L, NdotL) * e->apply(ls.L) * NdotL;

			MSI::balance(semi_weight, semi_pdf, weight, lightSampleWeight * ls.PDF_S);
		}

		MSI::balance(full_weight, full_pdf, semi_weight,
			inflightCountWeight * (std::isinf(semi_pdf) ? 1 : semi_pdf));
	}

	return full_weight;
}

Spectrum Integrator::handleSpecularPath(const Ray& in, const ShaderClosure& sc, RenderTile* tile, RenderEntity*& lastEntity)
{
	ShaderClosure other_sc;
	Ray ray = in;

	lastEntity = mRenderer->shoot(ray, other_sc, tile);

	if (lastEntity && other_sc.Material) {
		float NdotL		= std::max(0.0f, ray.direction().dot(other_sc.N));
		Spectrum weight = other_sc.Material->eval(other_sc, ray.direction(), NdotL) * NdotL;

		for (uint32 depth = in.depth();
			 depth < renderer()->settings().maxRayDepth();
			 ++depth) {
			MaterialSample ms = other_sc.Material->sample(other_sc,
															tile->random().get3D());

			if (!std::isinf(ms.PDF_S))
				break;

			float NdotL = std::max(0.0f, ray.direction().dot(other_sc.N));
			if (NdotL <= PR_EPSILON)
				break;

			ray = ray.next(other_sc.P, ms.L);

			lastEntity = mRenderer->shoot(ray, other_sc, tile);
			if (lastEntity && other_sc.Material)
				weight *= other_sc.Material->eval(other_sc, ms.L, NdotL) * NdotL;
			else
				break;
		}

		return weight;
	} else {
		return Spectrum(1);
	}
}
}
