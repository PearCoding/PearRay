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
				delete[] mThreadData[i].CameraDiff;
				delete[] mThreadData[i].LightPath;
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
				delete[] mThreadData[i].CameraFacePoints;
				delete[] mThreadData[i].CameraDiff;
				delete[] mThreadData[i].LightPath;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;
		}

		mThreadCount = renderer->threads();
		mThreadData = new ThreadData[renderer->threads()];

		if (!mThreadData || renderer->lights().empty() || renderer->settings().maxLightSamples() == 0)
			return;

		size_t lightSampleSize = renderer->settings().maxRayDepth() * renderer->lights().size() * renderer->settings().maxLightSamples();
		for (uint32 i = 0; i < mThreadCount; ++i)
		{
			mThreadData[i].CameraPath = new Ray[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraEntities = new RenderEntity*[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraFacePoints = new FacePoint[renderer->settings().maxRayDepth()];
			mThreadData[i].CameraDiff = new bool[renderer->settings().maxRayDepth()];
			mThreadData[i].LightPath = new Ray[lightSampleSize];
			//mThreadData[i].LightEntities = new RenderEntity*[lightSampleSize];
			mThreadData[i].LightMaxDepth = new uint32[renderer->lights().size() * renderer->settings().maxLightSamples()];
		}
	}

	/* Attention: Don't use shootWithApply... it will call this again and destroy the thread data! */
	/* This integrator should only be called from top */

	constexpr float NormalOffset = 0.001f;
	constexpr float LightEpsilon = 0.0001f;
	Spectrum BiDirectIntegrator::apply(Ray& in, RenderEntity* initialEntity, const FacePoint& point, RenderContext* context)
	{
		if (!mThreadData || context->renderer()->lights().empty() || context->renderer()->settings().maxLightSamples() == 0)
			return Spectrum();

		Renderer* renderer = context->renderer();
		ThreadData& data = mThreadData[context->threadNumber()];

		Ray current = in;
		uint32 depth = 0;

		data.CameraEntities[depth] = initialEntity;
		data.CameraFacePoints[depth] = point;
		data.CameraPath[depth] = current;
		data.CameraDiff[depth] = initialEntity->material()->roughness(point) > PM_EPSILON;

		// First let the camera ray travel!
		for (depth = 1; depth < renderer->settings().maxRayDepth(); ++depth)
		{
			FacePoint collision;
			RenderEntity* entity = context->shoot(current, collision);
			if (entity && entity->material())
			{
				bool store;
				if (!handleObject(current, entity, collision, renderer, false, store))
					break;

				data.CameraEntities[depth] = entity;
				data.CameraFacePoints[depth] = collision;
				data.CameraPath[depth] = current;
				data.CameraDiff[depth] = store;
			}
			else
			{
				break;
			}
		}

		depth -= 1; // Drop last one... it is not useful

		// Next let the lights samples travel!
		RandomSampler sampler(renderer->random());
		uint32 lightNr = 0;
		for (RenderEntity* light : renderer->lights())
		{
			for (uint32 i = 0; i < renderer->settings().maxLightSamples(); ++i)
			{
				Ray* lightRays = &data.LightPath[lightNr * renderer->settings().maxRayDepth()];
				//RenderEntity** lightEntities = &data.LightEntities[lightNr * renderer->settings().maxRayDepth()];

				FacePoint lightSample = light->getRandomFacePoint(sampler, renderer->random());
				lightSample.setNormal(
					Projection::align(lightSample.normal(),
						Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat(), 2)));

				current.setStartPosition(PM::pm_Add(lightSample.vertex(), PM::pm_Scale(lightSample.normal(), NormalOffset)));
				current.setDirection(lightSample.normal());
				current.setDepth(1);

				// Initiate with power
				current.setSpectrum(light->material()->applyEmission(lightSample, lightSample.normal()));

				uint32 lightDepth = 0;
				lightRays[lightDepth] = current;
				//lightEntities[lightDepth] = light;
				for (uint32 k = 1; k < renderer->settings().maxRayDepth(); ++k)
				{
					FacePoint collision;
					RenderEntity* entity = context->shoot(current, collision);
					if (entity && entity->material())
					{
						bool store;
						if (!handleObject(current, entity, collision, renderer, true, store))
							break;

						if (store)
						{
							//lightEntities[lightDepth] = entity;
							lightRays[lightDepth] = current;
							lightDepth++;
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

		// Now sample all possible paths together!
		Spectrum lastSpec;
		for (int32 i = depth; i >= 0; --i)// Reverse!
		{
			RenderEntity* entity = data.CameraEntities[i];
			const Ray& cameraVertex = data.CameraPath[i];

			Spectrum newSpec;
			uint32 samples = 0;

			if (data.CameraDiff[i])
			{
				for (uint32 j = 0; j < lightNr; ++j)// Each light
				{
					for (uint32 s = 0; s < data.LightMaxDepth[j]; ++s)
					{
						const Ray& lightVertex = data.LightPath[j * renderer->settings().maxRayDepth() + s];

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
									lightVertex.spectrum()) * NdotL;
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

			if (samples > 0)
				lastSpec = PM_PI_F * newSpec / (float)samples;

			lastSpec += cameraVertex.spectrum();
		}

		return lastSpec;
	}

	bool BiDirectIntegrator::handleObject(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer, bool applyMat, bool& store)
	{
		PM::vec3 nextDir;

		// Russian Roulette
		float rnd = renderer->random().getFloat();
		const float roughness = entity->material()->roughness(point);
		if (!entity->material()->canBeShaded() || rnd < roughness)// Diffuse
		{
			nextDir = PM::pm_SetW(Projection::align(point.normal(),
				Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat(), 2)), 0);
			store = true;
		}
		else// Reflect
		{
			store = false;

			PM::vec3 traV;
			PM::vec3 reflV;

			float reflWeight = entity->material()->emitReflectionVector(point, in.direction(), reflV);// Not pointN!
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

		const float NdotL = std::abs(PM::pm_Dot3D(point.normal(), nextDir));

		if (NdotL < PM_EPSILON)
			return false;

		if (applyMat)
			in.setSpectrum(entity->material()->apply(point, in.direction(), nextDir, in.spectrum())*NdotL);
		else if (entity->material()->isLight())
			in.setSpectrum(in.spectrum() + entity->material()->applyEmission(point, in.direction()));

		in.setDirection(nextDir);
		in.setStartPosition(PM::pm_Add(point.vertex(), PM::pm_Scale(nextDir, NormalOffset)));
		in.setDepth(in.depth() + 1);
		return true;
	}
}