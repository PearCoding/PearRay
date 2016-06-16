#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "sampler/MultiJitteredSampler.h"
#include "material/Material.h"

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
			MultiJitteredSampler sampler(context->renderer()->random(), context->renderer()->settings().maxLightSamples());
			for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
			{
				FacePoint p = light->getRandomFacePoint(sampler, context->renderer()->random(), i);

				const PM::vec3 L = PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex()));
				const float NdotL = PM::pm_Dot3D(L, point.normal());

				float pdf = point.material()->pdf(point, in.direction(), L);
				if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
				{
					if (!std::isinf(pdf))
					{
						FacePoint tmpPoint;
						Spectrum applied;
						Ray ray(point.vertex(), L, in.depth() + 1);
						ray.setFlags(ray.flags() | RF_ShadowRay);

						if (context->shootWithApply(applied, ray, tmpPoint) == light)// Full light!!
						{
							spec += point.material()->apply(point, in.direction(), L) * applied *
								(NdotL / pdf);
						}
					}
					else// Specular
					{
						PM::vec3 rnd = PM::pm_Set(context->renderer()->random().getFloat(),
							context->renderer()->random().getFloat(),
							context->renderer()->random().getFloat());
						PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf);
						Ray out;
						out.setDepth(in.depth() + 1);
						out.setDirection(dir);
						out.setStartPosition(point.vertex());

						PR_ASSERT(std::isinf(pdf));

						FacePoint point2;
						Spectrum applied;
						RenderEntity* entity2 = context->shootWithApply(applied, out, point2);

						if (!entity2 || !point2.material() || !point2.material()->canBeShaded())
							return applied;

						return applied + point.material()->apply(point, in.direction(), out.direction()) * applyRay(out, point2, entity2, context) * std::abs(PM::pm_Dot3D(point.normal(), out.direction()));
					}
				}
				sampleCounter++;
			}
		}

		return spec * (PM_PI_F / sampleCounter);
	}
}