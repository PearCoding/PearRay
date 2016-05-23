#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "sampler/Projection.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "sampler/StratifiedSampler.h"
#include "material/Material.h"

namespace PR
{
	DirectIntegrator::DirectIntegrator(Renderer* renderer) : Integrator(),
		mLightSampler(renderer->random(), renderer->settings().maxLightSamples())
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

		if (!entity || !entity->material() || !entity->material()->canBeShaded())
			return applied;
		
		return applied + applyRay(in, point, entity, context);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const FacePoint& point, RenderEntity* entity, RenderContext* context)
	{
		if (in.depth() >= context->renderer()->settings().maxRayDepth() ||
			!entity->material()->canBeShaded())
			return Spectrum();

		float rnd = context->renderer()->random().getFloat();

		if (rnd < entity->material()->roughness(point))// Diffuse
		{
			uint32 sampleCounter = 0;

			Spectrum spec;
			for (RenderEntity* light : context->renderer()->lights())
			{
				for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
				{
					FacePoint p = light->getRandomFacePoint(mLightSampler, context->renderer()->random());

					const PM::vec3 L = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);
					const float NdotL = std::abs(PM::pm_Dot3D(L, point.normal()));

					if (NdotL >= PM_EPSILON)
					{
						FacePoint tmpPoint;
						Spectrum applied;
						Ray ray(point.vertex(), L, in.depth() + 1);
						ray.setFlags(ray.flags() | RF_ShadowRay);

						if (context->shootWithApply(applied, ray, tmpPoint) == light)// Full light!!
						{
							spec += entity->material()->apply(point, in.direction(), L, applied)*NdotL;
						}

					}
					sampleCounter++;
				}
			}

			return spec * (PM_PI_F / sampleCounter);
		}
		else // Specular
		{
			PM::vec3 reflectionVector;
			PM::vec3 transmissionVector;
			float refWeight = entity->material()->emitReflectionVector(point, in.direction(), reflectionVector);
			float transWeight = entity->material()->emitTransmissionVector(point, in.direction(), transmissionVector);

			PR_DEBUG_ASSERT(refWeight >= 0 && transWeight >= 0 && refWeight + transWeight <= 1);

			rnd = context->renderer()->random().getFloat();

			if (rnd < refWeight)
			{
				FacePoint tmpPoint;
				Ray ray(point.vertex(),	reflectionVector, in.depth() + 1);

				Spectrum applied;
				RenderEntity* newEntity = context->shootWithApply(applied, ray, tmpPoint);
				if (newEntity && newEntity->material())
				{
					const float NdotL = std::abs(PM::pm_Dot3D(reflectionVector, point.normal()));
					return applied + entity->material()->apply(point, in.direction(), reflectionVector,
						applyRay(ray, tmpPoint, newEntity, context)) * NdotL;
				}
			}
			else if(rnd < refWeight + transWeight)
			{
				FacePoint tmpPoint;
				Ray ray(point.vertex(),	transmissionVector, in.depth() + 1);

				Spectrum applied;
				RenderEntity* newEntity = context->shootWithApply(applied, ray, tmpPoint);
				if (newEntity && newEntity->material())
				{
					const float NdotL = std::abs(PM::pm_Dot3D(transmissionVector, point.normal()));
					return applied + entity->material()->apply(point, in.direction(), transmissionVector,
						applyRay(ray, tmpPoint, newEntity, context)) * NdotL;
				}
			}

			return Spectrum();
		}
	}
}