#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
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
		SamplePoint point;
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, point);

		if (!entity || !point.Material)
			return applied;
		
		return applied + applyRay(in, point, context);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const SamplePoint& point, RenderContext* context)
	{
		if (!point.Material->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Hemisphere sampling
		RandomSampler hemiSampler(context->random());
		for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples() && !std::isinf(full_pdf); ++i)
		{
			float pdf;
			Spectrum weight;
			PM::vec3 rnd = hemiSampler.generate3D(i);
			PM::vec3 dir = point.Material->sample(point, rnd, pdf);
			const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(dir, point.N));

			if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
			{
				Ray ray = in.next(point.P, dir);

				SamplePoint point2;
				Spectrum applied;
				if (context->shootWithEmission(applied, ray, point2) && point2.Material && std::isinf(pdf))
					applied += applyRay(ray, point2, context);

				weight = point.Material->apply(point, ray.direction()) * applied * NdotL;
			}

			MSI::balance(full_weight, full_pdf, weight, pdf);
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
					SamplePoint p = light->getRandomFacePoint(sampler, i, pdf);

					const PM::vec3 PS = PM::pm_Subtract(p.P, point.P);
					const PM::vec3 L = PM::pm_Normalize3D(PS);
					const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, point.N));

					pdf = MSI::toSolidAngle(pdf, PM::pm_MagnitudeSqr3D(PS), NdotL);

					Spectrum weight;
					pdf += point.Material->pdf(point, L);
					if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
					{
						SamplePoint tmpPoint;
						Spectrum applied;
						Ray ray = in.next(point.P, L);

						if (context->shootWithEmission(applied, ray, tmpPoint) == light)// Full light!!
							weight = point.Material->apply(point, L) * applied * NdotL;
					}

					MSI::balance(full_weight, full_pdf, weight, pdf);
				}
			}
		}

		return full_weight;
	}
}