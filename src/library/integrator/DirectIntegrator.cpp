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
	DirectIntegrator::DirectIntegrator(Renderer* renderer, uint32 lightSamples) : Integrator(),
		mLightSamples(lightSamples), mLightSampler(renderer->random(), lightSamples)
	{
	}

	void DirectIntegrator::init(Renderer* renderer)
	{
	}

	constexpr float NormalOffset = 0.0001f;
	Spectrum DirectIntegrator::apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context)
	{
		if (!entity->material()->canBeShaded())
			return Spectrum();

		const PM::vec3 N = PM::pm_SetW(point.normal(), 0);
		FacePoint collisionPoint;
		Spectrum spec;

		bool onlySpecular = entity->material()->roughness(point) <= PM_EPSILON;

		PM::vec3 reflectionVector;
		PM::vec3 transmissionVector;
		float refWeight, transWeight;
		
		if (onlySpecular)
		{
			refWeight = entity->material()->emitReflectionVector(point, in.direction(), reflectionVector);
			transWeight = entity->material()->emitTransmissionVector(point, in.direction(), transmissionVector);

			onlySpecular = (refWeight + transWeight) > PM_EPSILON;
		}

		if (!onlySpecular)
		{
			uint32 sampleCounter = 0;

			for (RenderEntity* light : context->renderer()->lights())
			{				
				for (uint32 i = 0; i < mLightSamples; ++i)
				{
					FacePoint p = light->getRandomFacePoint(mLightSampler, context->renderer()->random());

					const PM::vec3 L = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);
					const float NdotL = std::abs(PM::pm_Dot3D(L, N));

					if (NdotL > PM_EPSILON)
					{
						Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(L, NormalOffset)), L, in.depth() + 1);
						ray.setFlags(ray.flags() & RF_ShadowRay);

						if (context->shootWithApply(ray, collisionPoint) == light)// Full light!!
						{
							spec += entity->material()->apply(point, in.direction(), L, ray.spectrum())*NdotL;
						}
							
						sampleCounter++;
					}
				}
			}

			if (sampleCounter != 0)
			{
				spec *= PM_PI_F / sampleCounter;
			}
		}
		else
		{
			if (refWeight > PM_EPSILON)
			{
				Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(reflectionVector, NormalOffset)),
					reflectionVector, in.depth() + 1);

				if (context->shootWithApply(ray, collisionPoint))
				{
					const float NdotL = std::abs(PM::pm_Dot3D(reflectionVector, N));
					spec = entity->material()->apply(point, in.direction(), reflectionVector, ray.spectrum()) * NdotL * refWeight;
				}
			}

			const float NdotL_trans = std::abs(PM::pm_Dot3D(transmissionVector, N));
			if (transWeight > PM_EPSILON && NdotL_trans > PM_EPSILON)
			{
				Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(transmissionVector, NormalOffset)),
					transmissionVector, in.depth() + 1);

				if (context->shootWithApply(ray, collisionPoint))
				{
					spec += entity->material()->apply(point, in.direction(), transmissionVector, ray.spectrum()) *
						(transWeight * NdotL_trans);
				}
			}
		}
		
		return spec;
	}
}