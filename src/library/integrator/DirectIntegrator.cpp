#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "sampler/MultiJitteredSampler.h"
#include "material/Material.h"
#include "math/MSI.h"

namespace PR
{
	DirectIntegrator::DirectIntegrator(Renderer* renderer) : Integrator()
	{
	}

	void DirectIntegrator::init(Renderer* renderer)
	{
	}

	Spectrum DirectIntegrator::apply(const Ray& in, RenderContext* context)
	{
		FacePoint point;
		Spectrum applied;
		RenderEntity* entity = context->shootWithApply(applied, in, point);

		if (!entity || !point.material() || !point.material()->canBeShaded() ||
			context->renderer()->settings().maxLightSamples() == 0)
			return applied;
		
		return applied + applyRay(in, point, context);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const FacePoint& point, RenderContext* context)
	{
		if (!point.material()->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Hemisphere sampling
		MultiJitteredSampler hemiSampler(context->random(), context->renderer()->settings().maxLightSamples());
		for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples() && !std::isinf(full_pdf); ++i)
		{
			float pdf;
			Spectrum weight;
			PM::vec3 rnd = hemiSampler.generate3D(i);
			PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf);
			const float NdotL = std::abs(PM::pm_Dot3D(dir, point.normal()));

			if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
			{
				Ray ray(point.vertex(), dir, in.depth() + 1);

				FacePoint point2;
				Spectrum applied;
				if (context->shootWithApply(applied, ray, point2) && point2.material() && std::isinf(pdf))
				{
					applied += applyRay(ray, point2, context);
				}

				weight = point.material()->apply(point, in.direction(), ray.direction()) * applied * NdotL;
			}
			else
			{
				pdf = 0;
			}

			MSI::balance(full_weight, full_pdf, weight, pdf);
		}

		if (!std::isinf(full_pdf))
		{
			// Area sampling!
			for (RenderEntity* light : context->renderer()->lights())
			{
				MultiJitteredSampler sampler(context->random(), context->renderer()->settings().maxLightSamples());
				for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
				{
					FacePoint p = light->getRandomFacePoint(sampler, i);

					const PM::vec3 L = PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex()));
					const float NdotL = PM::pm_Dot3D(L, point.normal());

					Spectrum weight;
					float pdf = point.material()->pdf(point, in.direction(), L);
					if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
					{
						FacePoint tmpPoint;
						Spectrum applied;
						Ray ray(point.vertex(), L, in.depth() + 1);
						ray.setFlags(ray.flags() | RF_ShadowRay);

						if (context->shootWithApply(applied, ray, tmpPoint) == light)// Full light!!
						{
							weight = point.material()->apply(point, in.direction(), L) * applied * NdotL;
						}
					}

					MSI::balance(full_weight, full_pdf, weight, pdf);
				}
			}
		}

		return full_weight;
	}
}