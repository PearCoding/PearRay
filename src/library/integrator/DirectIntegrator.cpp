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

		if (!entity || !point.material() || !point.material()->canBeShaded())
			return applied;
		
		return applied + applyRay(in, point, entity, context);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const FacePoint& point, RenderEntity* entity, RenderContext* context)
	{
		if (!point.material()->canBeShaded())
			return Spectrum();

		uint32 sampleCounter = 0;

		Spectrum spec;
		for (RenderEntity* light : context->renderer()->lights())
		{
			// TODO: Put hemisphere sampling here!
			MultiJitteredSampler sampler(context->renderer()->random(), context->renderer()->settings().maxLightSamples());
			for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
			{
				// Area sampling!
				FacePoint p = light->getRandomFacePoint(sampler, context->renderer()->random(), i);

				const PM::vec3 L = PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex()));
				const float NdotL = PM::pm_Dot3D(L, point.normal());

				Spectrum weight1;
				float pdf1 = point.material()->pdf(point, in.direction(), L);
				if (NdotL > PM_EPSILON && pdf1 > PM_EPSILON)
				{
					if (!std::isinf(pdf1))
					{
						FacePoint tmpPoint;
						Spectrum applied;
						Ray ray(point.vertex(), L, in.depth() + 1);
						ray.setFlags(ray.flags() | RF_ShadowRay);

						if (context->shootWithApply(applied, ray, tmpPoint) == light)// Full light!!
						{
							spec += point.material()->apply(point, in.direction(), L) * applied * NdotL;
						}
					}
					else// Specular, breaking weights. No other positions possible.
					{
						PM::vec3 rnd = PM::pm_Set(context->renderer()->random().getFloat(),
							context->renderer()->random().getFloat(),
							context->renderer()->random().getFloat());
						PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf1);
						Ray out;
						out.setDepth(in.depth() + 1);
						out.setDirection(dir);
						out.setStartPosition(point.vertex());

						PR_ASSERT(std::isinf(pdf1));

						FacePoint point2;
						Spectrum applied;
						RenderEntity* entity2 = context->shootWithApply(applied, out, point2);

						if (!entity2 || !point2.material() || !point2.material()->canBeShaded())
							return applied;

						return applied + point.material()->apply(point, in.direction(), out.direction()) * applyRay(out, point2, entity2, context) * std::abs(PM::pm_Dot3D(point.normal(), out.direction()));
					}
				}
				else
				{
					pdf1 = 0;
				}

				float full_pdf = 0;
				Spectrum weight;
				MSI::balance(weight, full_pdf, weight1, pdf1, 0.5f);

				// Hemisphere sampling
				float pdf2;
				Spectrum weight2;
				PM::vec3 rnd = PM::pm_Set(context->renderer()->random().getFloat(),
					context->renderer()->random().getFloat(),
					context->renderer()->random().getFloat());
				PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf2);
				Ray out;
				out.setDepth(in.depth() + 1);
				out.setDirection(dir);
				out.setStartPosition(point.vertex());

				FacePoint point2;
				Spectrum applied;
				RenderEntity* entity2 = context->shootWithApply(applied, out, point2);

				if (entity2 && point2.material() && point2.material()->canBeShaded())
				{
					weight2 = point.material()->apply(point, in.direction(), out.direction()) * applied * std::abs(PM::pm_Dot3D(point.normal(), out.direction()));
				}
				else
				{
					pdf2 = 0;
				}
				MSI::balance(weight, full_pdf, weight2, pdf2, 0.5f);

				spec += 0.5f * weight;
				sampleCounter++;
			}
		}

		return spec * (PM_PI_F / sampleCounter);
	}
}