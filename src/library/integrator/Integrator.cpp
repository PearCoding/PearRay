#include "Integrator.h"

#include "light/IInfiniteLight.h"
#include "spectral/Spectrum.h"

#include "sampler/RandomSampler.h"

#include "renderer/RenderThreadContext.h"

#include "scene/Scene.h"

#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "ray/Ray.h"

#include "math/MSI.h"

namespace PR {
Spectrum Integrator::handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, float& full_pdf)
{
	Spectrum full_weight;
	full_pdf = 0;

	RandomSampler sampler(context->random());
	for (const auto& e : context->renderer()->scene().infiniteLights()) {
		Spectrum semi_weight;
		float semi_pdf = 0;

		for (uint32 i = 0;
			 i < context->renderer()->settings().maxLightSamples() && !std::isinf(semi_pdf);
			 ++i) {
			float pdf;
			Eigen::Vector3f dir = e->sample(sc, sampler.generate3D(i), pdf);

			if (pdf <= PR_EPSILON)
				continue;

			Spectrum weight;
			const float NdotL = std::abs(dir.dot(sc.N));

			if (NdotL > PR_EPSILON) {
				RenderEntity* entity;

				Ray ray = in.next(sc.P, dir);
				ray.setFlags(ray.flags() | RF_Light);

				weight = handleSpecularPath(ray, sc, context, entity);
				if (!entity)
					weight *= sc.Material->eval(sc, dir, NdotL) * e->apply(dir) * NdotL;
				else
					weight.clear();
			}

			MSI::balance(semi_weight, semi_pdf, weight, pdf);
		}

		MSI::balance(full_weight, full_pdf, semi_weight, std::isinf(semi_pdf) ? 1 : semi_pdf);
	}

	return full_weight;
}

Spectrum Integrator::handleSpecularPath(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, RenderEntity*& lastEntity)
{
	ShaderClosure other_sc;
	Ray ray = in;

	lastEntity = context->shoot(ray, other_sc);

	if (lastEntity && other_sc.Material) {
		float NdotL		= std::max(0.0f, ray.direction().dot(other_sc.N));
		Spectrum weight = other_sc.Material->eval(other_sc, ray.direction(), NdotL) * NdotL;

		float other_pdf;
		for (uint32 depth = in.depth();
			 depth < context->renderer()->settings().maxRayDepth();
			 ++depth) {
			Eigen::Vector3f dir = other_sc.Material->sample(other_sc,
															context->random().get3D(),
															other_pdf);

			if (!std::isinf(other_pdf))
				break;

			float NdotL = std::max(0.0f, ray.direction().dot(other_sc.N));
			if (NdotL <= PR_EPSILON)
				break;

			ray = ray.next(other_sc.P, dir);

			lastEntity = context->shoot(ray, other_sc);
			if (lastEntity && other_sc.Material)
				weight *= other_sc.Material->eval(other_sc, dir, NdotL) * NdotL;
			else
				break;
		}

		return weight;
	} else {
		return Spectrum(1);
	}
}
}
