#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "entity/RenderEntity.h"
#include "sampler/RandomSampler.h"
#include "material/Material.h"
#include "math/MSI.h"

namespace PR
{
	DirectIntegrator::DirectIntegrator(RenderContext* renderer) : OnePassIntegrator()
	{
	}

	void DirectIntegrator::init(RenderContext* renderer)
	{
	}

	Spectrum DirectIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
	{
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		if (!entity || !sc.Material)
			return applied;
		
		return applied + applyRay(in, sc, context, 0);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, uint32 diffbounces)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Used temporary
		ShaderClosure other_sc;

		// Hemisphere sampling
		RandomSampler hemiSampler(context->random());
		for (uint32 i = 0;
			 i < context->renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
			 ++i)
		{
			Spectrum other_weight;
			float other_pdf = 0;
			PM::vec3 rnd = hemiSampler.generate3D(i);
			for(uint32 path = 0; path < sc.Material->samplePathCount(); ++path)
			{
				float path_weight;
				float pdf;

				PM::vec3 dir = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);
				if(path_weight <= PM_EPSILON)
					continue;
				
				const float NdotL = std::abs(PM::pm_Dot3D(dir, sc.N));

				if (pdf > PM_EPSILON)
				{
					if (NdotL > PM_EPSILON)
					{
						Spectrum weight;
						Ray ray = in.next(sc.P, dir);

						RenderEntity* entity = context->shootWithEmission(weight, ray, other_sc);
						if (entity && other_sc.Material &&
							(std::isinf(pdf) || diffbounces < context->renderer()->settings().maxDiffuseBounces()))						
							weight += applyRay(ray, other_sc, context,
								!std::isinf(pdf) ? diffbounces + 1 : diffbounces);

						weight *= sc.Material->eval(sc, dir, NdotL) * NdotL;
						other_weight += path_weight*weight;
					}					
				}

				other_pdf += path_weight * pdf;
			}
			MSI::power(full_weight, full_pdf, other_weight, other_pdf);
		}

		if (!std::isinf(full_pdf))
		{
			Spectrum other_weight;
			// Area sampling!
			for (RenderEntity* light : context->renderer()->lights())
			{
				RandomSampler sampler(context->random());
				for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
				{
					float pdf;
					FaceSample p = light->getRandomFacePoint(sampler, i, pdf);

					const PM::vec3 PS = PM::pm_Subtract(p.P, sc.P);
					const PM::vec3 L = PM::pm_Normalize3D(PS);
					const float NdotL = PM::pm_Max(0.0f, PM::pm_Dot3D(L, sc.N));

					pdf = MSI::toSolidAngle(pdf, PM::pm_MagnitudeSqr3D(PS), NdotL) 
						+ sc.Material->pdf(sc, L, NdotL);

					if (pdf > PM_EPSILON)
					{
						if (NdotL > PM_EPSILON)
						{
							Ray ray = in.next(sc.P, L);
							if (context->shootWithEmission(other_weight, ray, other_sc) == light)// Full light!!
								other_weight *= sc.Material->eval(sc, L, NdotL) * NdotL;
							else
								other_weight.clear();
						}
						else
							other_weight.clear();

						MSI::power(full_weight, full_pdf,
							other_weight, std::isinf(pdf) ? 1 : pdf);
					}
				}
			}

			float inf_pdf;
			other_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::power(full_weight, full_pdf, other_weight, inf_pdf);
		}

		return full_weight;
	}
}