#include "BiDirectIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "material/Material.h"

#include "sampler/RandomSampler.h"
#include "sampler/Projection.h"

#include "photon/PhotonMap.h"

namespace PR
{
	BiDirectIntegrator::BiDirectIntegrator() :
		Integrator(), mThreadData(nullptr), mThreadCount(0)
	{

	}

	BiDirectIntegrator::~BiDirectIntegrator()
	{
		if (mThreadData)
		{
			for (uint32 i = 0; i < mThreadCount; ++i)
			{
				delete[] mThreadData[i].LightPos;
				delete[] mThreadData[i].LightFlux;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;
		}
	}

	void BiDirectIntegrator::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);

		if (mThreadData)
		{
			for (uint32 i = 0; i < mThreadCount; ++i)
			{
				delete[] mThreadData[i].LightPos;
				delete[] mThreadData[i].LightFlux;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;
		}

		mThreadCount = renderer->threads();
		mThreadData = new ThreadData[renderer->threads()];
		size_t maxlightsamples = renderer->settings().maxRayDepth() *
			renderer->lights().size() * renderer->settings().maxLightSamples();

		if (!mThreadData || renderer->lights().empty() || renderer->settings().maxLightSamples() == 0)
			return;

		for (uint32 i = 0; i < mThreadCount; ++i)
		{
			mThreadData[i].LightPos = new float[maxlightsamples*3];
			mThreadData[i].LightFlux = new Spectrum[maxlightsamples];
			//mThreadData[i].LightEntities = new RenderEntity*[mMaxLightSampleCount];
			mThreadData[i].LightMaxDepth = new uint32[renderer->lights().size() * renderer->settings().maxLightSamples()];
		}
	}
	
	constexpr float LightEpsilon = 0.00001f;
	Spectrum BiDirectIntegrator::apply(const Ray& in, RenderContext* context)
	{
		if (!mThreadData || context->renderer()->lights().empty() || context->renderer()->settings().maxLightSamples() == 0)
			return Spectrum();


		Renderer* renderer = context->renderer();
		ThreadData& data = mThreadData[context->threadNumber()];

		const uint32 maxDepth = renderer->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer->settings().maxDiffuseBounces();

		RandomSampler sampler(renderer->random());
		Ray current = in;
		uint32 lightNr = 0;
		for (RenderEntity* light : renderer->lights())
		{
			for (uint32 i = 0; i < renderer->settings().maxLightSamples(); ++i)
			{
				float* lightPos = &data.LightPos[lightNr * maxDepth];
				Spectrum* lightFlux = &data.LightFlux[lightNr * maxDepth];
				//RenderEntity** lightEntities = &data.LightEntities[lightNr * renderer->settings().maxRayDepth()];

				FacePoint lightSample = light->getRandomFacePoint(sampler, renderer->random());
				lightSample.setNormal(
					Projection::align(lightSample.normal(),
						Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat())));

				current.setStartPosition(lightSample.vertex());
				current.setDirection(lightSample.normal());
				current.setDepth(1);

				// Initiate with power
				Spectrum flux = light->material()->applyEmission(lightSample, lightSample.normal());

				uint32 lightDepth = 0;// Counts diff bounces
				PM::pm_Store3D(current.startPosition(), &lightPos[lightDepth*3]);
				lightFlux[lightDepth] = flux;

				//lightEntities[lightDepth] = light;
				for (uint32 k = 1; k < maxDepth && lightDepth <= maxDiffBounces; ++k)
				{
					FacePoint collision;
					RenderEntity* entity = context->shoot(current, collision);
					if (entity && entity->material() && entity->material()->canBeShaded())
					{
						const float NdotL = std::abs(PM::pm_Dot3D(collision.normal(), current.direction()));
						if (NdotL < PM_EPSILON)
							break;

						bool store;
						Ray out;
						if (!handleObject(current, out, entity, collision, renderer, store))
							break;

						flux = entity->material()->apply(collision, out.direction(), current.direction(), flux) * NdotL;
						current = out;

						if (store)
						{
							lightDepth++;
							//lightEntities[lightDepth] = entity;
							lightFlux[lightDepth] = flux;
							PM::pm_Store3D(current.startPosition(), &lightPos[lightDepth * 3]);
						}
					}
					else
					{
						break;
					}

				}

				data.LightMaxDepth[lightNr] = lightDepth;
				lightNr++;
			}
		}

		return applyRay(in, context, 0);
	}

	Spectrum BiDirectIntegrator::applyRay(const Ray& in, RenderContext* context, uint32 diffBounces)
	{
		const uint32 maxLights = context->renderer()->settings().maxLightSamples()*context->renderer()->lights().size();
		const uint32 maxDepth = context->renderer()->settings().maxRayDepth();

		const ThreadData& data = mThreadData[context->threadNumber()];

		FacePoint collision;
		Spectrum applied;
		Spectrum spec;
		uint32 samples = 0;

		RenderEntity* entity = context->shootWithApply(applied, in, collision);
		if (entity && entity->material())
		{
			if (entity->material()->canBeShaded())
			{
				Ray out;
				bool diff;
				if (!handleObject(in, out, entity, collision, context->renderer(), diff))
					return applied;

				if (diff)
				{
					for (uint32 j = 0; j < maxLights; ++j)// Each light
					{
						for (uint32 s = 0; s < data.LightMaxDepth[j]; ++s)
						{
							const float* lightPosP = &data.LightPos[(j * maxDepth + s)*3];
							PM::vec3 lightPos = PM::pm_Set(lightPosP[0], lightPosP[1], lightPosP[2], 1);
							const Spectrum& lightFlux = data.LightFlux[j * maxDepth + s];

							Ray shootRay(lightPos,
								PM::pm_Normalize3D(PM::pm_Subtract(collision.vertex(), lightPos)));

							FacePoint tmpCollision;
							if (context->shoot(shootRay, tmpCollision) == entity &&
								PM::pm_MagnitudeSqr3D(PM::pm_Subtract(collision.vertex(), tmpCollision.vertex())) <= LightEpsilon)
							{
								const float NdotL = std::abs(PM::pm_Dot3D(tmpCollision.normal(), shootRay.direction()));
								if (NdotL > PM_EPSILON)
								{
									spec += entity->material()->apply(tmpCollision,
										in.direction(), shootRay.direction(),
										lightFlux)* NdotL;
									PR_DEBUG_ASSERT(!newSpec.hasNaN());
								}
							}
							samples++;
						}
					}
				}

				if (in.depth() < maxDepth && 
					(!diff || diffBounces <= context->renderer()->settings().maxDiffuseBounces()))
				{
					const float NdotL = std::abs(PM::pm_Dot3D(collision.normal(), out.direction()));
					if (NdotL > PM_EPSILON)
					{
						spec += entity->material()->apply(collision, in.direction(),
							out.direction(),
							applyRay(out, context, diff ? diffBounces + 1 : diffBounces)) * NdotL;
						PR_DEBUG_ASSERT(!newSpec.hasNaN());
					}
					samples++;
				}

				if (samples > 0)
					return applied + (PM_PI_F / (float)samples) * spec;
				else
					return applied;
			}
		}

		return applied;
	}

	bool BiDirectIntegrator::handleObject(const Ray& in, Ray& out, RenderEntity* entity, const FacePoint& point, Renderer* renderer, bool& store)
	{
		PM::vec3 nextDir;

		// Russian Roulette
		float rnd = renderer->random().getFloat();
		const float roughness = entity->material()->roughness(point);
		if (rnd < roughness)// Diffuse
		{
			nextDir = PM::pm_SetW(Projection::align(point.normal(),
				Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat())), 0);
			store = true;
		}
		else// Reflect
		{
			store = false;

			PM::vec3 traV;
			PM::vec3 reflV;

			float reflWeight = entity->material()->emitReflectionVector(point, in.direction(), reflV);
			float traWeight = entity->material()->emitTransmissionVector(point, in.direction(), traV);

			rnd = renderer->random().getFloat();

			if (rnd < reflWeight)
			{
				nextDir = reflV;
			}
			else if (rnd < reflWeight + traWeight)
			{
				nextDir = traV;
			}
			else
			{
				return false;
			}
		}

		out.setDirection(nextDir);
		out.setStartPosition(point.vertex());
		out.setDepth(in.depth() + 1);
		return true;
	}
}