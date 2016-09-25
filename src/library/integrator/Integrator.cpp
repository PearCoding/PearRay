#include "Integrator.h"

#include "spectral/Spectrum.h"
#include "light/IInfiniteLight.h"

#include "sampler/RandomSampler.h"

#include "renderer/RenderContext.h"

#include "scene/Scene.h"

#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "ray/Ray.h"

#include "math/MSI.h"

namespace PR
{
	Spectrum Integrator::handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderContext* context, float& full_pdf)
	{
		Spectrum full_weight;
		full_pdf = 0;

		RandomSampler sampler(context->random());
		for(IInfiniteLight* e : context->renderer()->scene()->infiniteLights())
		{
			Spectrum semi_weight;
			float semi_pdf = 0;

			for(uint32 i = 0;
				i < context->renderer()->settings().maxLightSamples() && !std::isinf(semi_pdf);
				++i)
			{
				float pdf;
				PM::vec3 rnd = sampler.generate3D(i);
				PM::vec3 dir = e->sample(sc, rnd, pdf);

				if(pdf > PM_EPSILON)
				{
					Spectrum weight;
					const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(dir, sc.N));

					if (NdotL > PM_EPSILON)
					{
						RenderEntity* entity;

						Ray ray = in.next(sc.P, dir);
						ray.setFlags(ray.flags() | RF_FromLight);

						weight = handleSpecularPath(ray, sc, context, entity);
						if (!entity)
							weight *= sc.Material->apply(sc, dir) * e->apply(dir) * NdotL;
						else
							weight.clear();
					}

					MSI::power(semi_weight, semi_pdf, weight, pdf);
				}
			}
			
			MSI::balance(full_weight, full_pdf, semi_weight, std::isinf(semi_pdf) ? 1 : semi_pdf);
		}

		return full_weight;
	}

	Spectrum Integrator::handleSpecularPath(const Ray& in, const ShaderClosure& sc, RenderContext* context, RenderEntity*& lastEntity)
	{
		ShaderClosure other_sc;
		Ray ray = in;

		lastEntity = context->shoot(ray, other_sc);

		if(lastEntity && other_sc.Material)
		{
			Spectrum weight = other_sc.Material->apply(other_sc, ray.direction());

			float other_pdf;
			for(uint32 depth = in.depth();
				depth < context->renderer()->settings().maxRayDepth();
				++depth)
			{
				PM::vec3 dir = other_sc.Material->sample(other_sc,
					PM::pm_Set(context->random().getFloat(), context->random().getFloat(), context->random().getFloat()),
					other_pdf);

				if(!std::isinf(other_pdf))
					break;

				const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(ray.direction(), other_sc.N));

				if (NdotL <= PM_EPSILON)
					break;
				
				ray = ray.next(other_sc.P, dir);

				lastEntity = context->shoot(ray, other_sc);
				if(lastEntity && other_sc.Material)
					weight *= other_sc.Material->apply(other_sc, dir) * NdotL;
				else
					break;
			}

			return weight; 
		}
		else
		{
			return Spectrum(1);
		}
	}
}