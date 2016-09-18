#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "sampler/RandomSampler.h"
#include "material/Material.h"
#include "math/MSI.h"

namespace PR
{
	DirectIntegrator::DirectIntegrator(Renderer* renderer) : OnePassIntegrator()
	{
	}

	void DirectIntegrator::init(Renderer* renderer)
	{
	}

	Spectrum DirectIntegrator::apply(const Ray& in, RenderContext* context, uint32 pass)
	{
		ShaderClosure sc;
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		if (!entity || !sc.Material)
			return applied;
		
		return applied + applyRay(in, sc, context);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const ShaderClosure& sc, RenderContext* context)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Used temporary
		ShaderClosure other_sc;
		Spectrum other_weight;

		// Hemisphere sampling
		RandomSampler hemiSampler(context->random());
		for (uint32 i = 0;
			 i < context->renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
			 ++i)
		{
			float pdf;
			PM::vec3 rnd = hemiSampler.generate3D(i);
			PM::vec3 dir = sc.Material->sample(sc, rnd, pdf);
			const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(dir, sc.N));

			if (pdf > PM_EPSILON)
			{
				if (NdotL > PM_EPSILON)
				{
					Ray ray = in.next(sc.P, dir);

					RenderEntity* entity = context->shootWithEmission(other_weight, ray, other_sc);
					if (entity && other_sc.Material && std::isinf(pdf))
						other_weight += applyRay(ray, other_sc, context);

					other_weight *= sc.Material->apply(sc, dir) * NdotL;
				}
				else
					other_weight.clear();
				
				MSI::power(full_weight, full_pdf, other_weight, pdf);
			}
		}

		if (!std::isinf(full_pdf))
		{
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
					const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, sc.N));

					//pdf = MSI::toSolidAngle(pdf, PM::pm_MagnitudeSqr3D(PS), NdotL) + sc.Material->pdf(sc, L);
					pdf += sc.Material->pdf(sc, L);

					if (pdf > PM_EPSILON)
					{
						if (NdotL > PM_EPSILON)
						{
							Ray ray = in.next(sc.P, L);
							if (context->shootWithEmission(other_weight, ray, other_sc) == light)// Full light!!
								other_weight *= sc.Material->apply(sc, L) * NdotL;
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
			MSI::power(full_weight, full_pdf,
				other_weight, std::isinf(inf_pdf) ? 1 : inf_pdf);
		}

		return full_weight;
	}
}