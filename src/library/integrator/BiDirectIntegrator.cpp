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
				delete[] mThreadData[i].CameraPath;
				delete[] mThreadData[i].CameraEntities;
				delete[] mThreadData[i].CameraFacePoints;
				delete[] mThreadData[i].CameraAffection;
				delete[] mThreadData[i].CameraDiff;
				delete[] mThreadData[i].LightPath;
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
				delete[] mThreadData[i].CameraPath;
				delete[] mThreadData[i].CameraEntities;
				delete[] mThreadData[i].CameraAffection;
				delete[] mThreadData[i].CameraFacePoints;
				delete[] mThreadData[i].CameraDiff;
				delete[] mThreadData[i].LightPath;
				delete[] mThreadData[i].LightFlux;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;
		}

		mThreadCount = renderer->threads();
		mThreadData = new ThreadData[renderer->threads()];

		if (!mThreadData || renderer->lights().empty() || renderer->settings().maxLightSamples() == 0)
			return;

		size_t lightSampleSize = renderer->settings().maxRayDepth() *
			renderer->lights().size() * renderer->settings().maxLightSamples();
		for (uint32 i = 0; i < mThreadCount; ++i)
		{
			mThreadData[i].CameraPath = new Ray[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraEntities = new RenderEntity*[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraFacePoints = new FacePoint[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraAffection = new Spectrum[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraDiff = new bool[renderer->settings().maxRayDepth()];
			mThreadData[i].LightPath = new Ray[lightSampleSize];
			mThreadData[i].LightFlux = new Spectrum[lightSampleSize];
			//mThreadData[i].LightEntities = new RenderEntity*[lightSampleSize];
			mThreadData[i].LightMaxDepth = new uint32[renderer->lights().size() * renderer->settings().maxLightSamples()];
		}
	}

	/* Attention: Don't use shootWithApply... it will call this again and destroy the thread data! */
	/* This integrator should only be called from top */

	constexpr float LightEpsilon = 0.00001f;
	Spectrum BiDirectIntegrator::apply(Ray& in, RenderContext* context)
	{
		if (!mThreadData || context->renderer()->lights().empty() || context->renderer()->settings().maxLightSamples() == 0)
			return Spectrum();


		Renderer* renderer = context->renderer();
		ThreadData& data = mThreadData[context->threadNumber()];

		const uint32 maxDepth = renderer->settings().maxRayDepth();
		const uint32 maxDiffBounces = renderer->settings().maxDiffuseBounces();

		Ray current = in;
		uint32 depth = 0;
		uint32 diffBounces = 0;

		// First let the camera ray travel!
		for (depth = 0; depth < maxDepth && diffBounces <= maxDiffBounces; ++depth)
		{
			FacePoint collision;
			Spectrum applied;
			RenderEntity* entity = context->shootWithApply(applied, current, collision);
			if (entity && entity->material())
			{
				if (entity->material()->canBeShaded())
				{
					bool store;
					if (!handleObject(current, entity, collision, renderer, store))
						break;

					data.CameraEntities[depth] = entity;
					data.CameraFacePoints[depth] = collision;
					data.CameraAffection[depth] = applied;
					data.CameraPath[depth] = current;
					data.CameraDiff[depth] = store;

					if (store)
						diffBounces++;
				}
				else// If not shadeable, then get the affect spec and stop
				{
					data.CameraEntities[depth] = entity;
					data.CameraFacePoints[depth] = collision;
					data.CameraAffection[depth] = applied;
					data.CameraPath[depth] = current;// Invalid
					data.CameraDiff[depth] = false;
					depth++;
					break;
				}
			}
			else
			{
				break;
			}
		}

		if (depth == 0)
			return Spectrum();

		depth -= 1; // Drop last one... it is not useful

		// Next let the lights samples travel!
		RandomSampler sampler(renderer->random());
		uint32 lightNr = 0;
		for (RenderEntity* light : renderer->lights())
		{
			for (uint32 i = 0; i < renderer->settings().maxLightSamples(); ++i)
			{
				Ray* lightRays = &data.LightPath[lightNr * maxDepth];
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

				uint32 lightDepth = 0;
				lightRays[lightDepth] = current;
				lightFlux[lightDepth] = flux;

				//lightEntities[lightDepth] = light;
				diffBounces = 0;
				for (uint32 k = 1; k < maxDepth && diffBounces <= maxDiffBounces; ++k)
				{
					FacePoint collision;
					RenderEntity* entity = context->shoot(current, collision);
					if (entity && entity->material())
					{
						PM::vec3 before = current.direction();

						const float NdotL = std::abs(PM::pm_Dot3D(collision.normal(), before));
						if (NdotL < PM_EPSILON)
							break;

						bool store;
						if (!handleObject(current, entity, collision, renderer, store))
							break;

						flux = entity->material()->apply(collision, current.direction(), before, flux);

						if (store)
						{
							lightDepth++;
							//lightEntities[lightDepth] = entity;
							lightFlux[lightDepth] = flux;
							lightRays[lightDepth] = current;
							diffBounces++;
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

		Spectrum lastSpec;
		// Now sample all possible paths together!
		for (int32 i = depth; i >= 0; --i)// Reverse!
		{
			RenderEntity* entity = data.CameraEntities[i];
			const Ray& cameraVertex = data.CameraPath[i];

			Spectrum newSpec;
			uint32 samples = 0;

			if (data.CameraDiff[i])// Had a diffuse reflection
			{
				for (uint32 j = 0; j < lightNr; ++j)// Each light
				{
					for (uint32 s = 0; s < data.LightMaxDepth[j]; ++s)
					{
						const Ray& lightVertex = data.LightPath[j * maxDepth + s];
						const Spectrum& lightFlux = data.LightFlux[j * maxDepth + s];

						Ray shootRay(lightVertex.startPosition(),
							PM::pm_Normalize3D(PM::pm_Subtract(cameraVertex.startPosition(), lightVertex.startPosition())));

						FacePoint tmpCollision;
						if (context->shoot(shootRay, tmpCollision) == entity &&
							PM::pm_MagnitudeSqr3D(PM::pm_Subtract(cameraVertex.startPosition(), tmpCollision.vertex())) <= LightEpsilon)
						{
							const float NdotL = std::abs(PM::pm_Dot3D(tmpCollision.normal(), shootRay.direction()));
							if (NdotL > PM_EPSILON)
							{
								newSpec += entity->material()->apply(tmpCollision,
									cameraVertex.direction(), shootRay.direction(),
									lightFlux)* NdotL;
								samples++;
							}
						}
					}
				}
			}

			// Apply also last spec
			if (i < depth)
			{
				float NdotL = std::abs(PM::pm_Dot3D(data.CameraFacePoints[i].normal(), data.CameraPath[i + 1].direction()));
				newSpec += entity->material()->apply(data.CameraFacePoints[i], cameraVertex.direction(),
						data.CameraPath[i + 1].direction(),
						lastSpec) * NdotL;
				samples++;
			}

			lastSpec = data.CameraAffection[i];
			if (samples > 0)
				lastSpec += PM_PI_F * newSpec / (float)samples;
		}

		return lastSpec;
	}

	bool BiDirectIntegrator::handleObject(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer, bool& store)
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

		in.setDirection(nextDir);
		in.setStartPosition(point.vertex());
		in.setDepth(in.depth() + 1);
		return true;
	}
}