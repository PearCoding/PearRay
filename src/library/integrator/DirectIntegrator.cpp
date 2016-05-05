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

		FacePoint collisionPoint;
		Spectrum diffuse;
		Spectrum spec;

		bool onlySpecular = entity->material()->roughness() <= PM_EPSILON;

		PM::vec3 reflectionVector;
		PM::vec3 transmissionVector;
		float refWeight, transWeight;
		
		if (onlySpecular)
		{
			refWeight = entity->material()->emitReflectionVector(point, in.direction(), reflectionVector);
			transWeight = entity->material()->emitTransmissionVector(point, in.direction(), transmissionVector);

			onlySpecular = (refWeight + transWeight) > 0;
		}

		if (!onlySpecular)
		{
			uint32 sampleCounter = 0;
			const PM::vec3 N = PM::pm_SetW(point.normal(), 0);

			for (RenderEntity* light : context->renderer()->lights())
			{
				const uint32 max = light->maxLightSamples() != 0 ? PM::pm_MinT(mLightSamples, light->maxLightSamples()) : mLightSamples;

				FacePoint p = light->getRandomFacePoint(mLightSampler, context->renderer()->random());

				const PM::vec3 L = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);
				const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, N));

				if (NdotL > std::numeric_limits<float>::epsilon())
				{
					Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(L, NormalOffset)), L, in.depth() + 1);
					ray.setFlags(ray.flags() & RF_ShadowRay);
					ray.setTarget(p.vertex());
					
					if (context->shootWithApply(ray, collisionPoint))// Full light!!
					{
						entity->material()->apply(point, in.direction(), L, ray.spectrum(), diffuse, spec);
						sampleCounter++;
					}
				}
			}

			if (sampleCounter != 0)
			{
				diffuse *= PM_PI_F / sampleCounter;
				spec *= PM_PI_F / sampleCounter;
			}
		}
		else
		{
			if (refWeight > 0)
			{
				Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(reflectionVector, NormalOffset)),
					reflectionVector, in.depth() + 1);

				if (context->shootWithApply(ray, collisionPoint))
				{			
					entity->material()->apply(point, in.direction(), reflectionVector, ray.spectrum(), diffuse, spec);

					const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(reflectionVector, point.normal()));
					diffuse *= NdotL * refWeight;
					spec *= NdotL * refWeight;
				}
			}

			const float NdotL_trans = std::abs(PM::pm_Dot3D(transmissionVector, point.normal()));
			if (transWeight > 0 && NdotL_trans > PM_EPSILON)
			{
				Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(transmissionVector, NormalOffset)),
					transmissionVector, in.depth() + 1);

				if (context->shootWithApply(ray, collisionPoint))
				{
					Spectrum diffuse2;
					Spectrum spec2;

					entity->material()->apply(point, in.direction(), transmissionVector, ray.spectrum(), diffuse2, spec2);
					diffuse += diffuse2 * (transWeight * NdotL_trans);
					spec += spec2 * (transWeight * NdotL_trans);
				}
			}
		}
		
		return diffuse + spec;
	}
}