#include "PhotonIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "material/Material.h"

#include "sampler/StratifiedSampler.h"
#include "sampler/Projection.h"

#include "photon/PhotonMap.h"

namespace PR
{
	PhotonIntegrator::PhotonIntegrator() :
		Integrator(), mMap(nullptr), mPhotonSpheres(nullptr), mSphereCount(0)
	{

	}

	PhotonIntegrator::~PhotonIntegrator()
	{
		if (mMap)
		{
			delete mMap;

			for (uint32 i = 0; i < mSphereCount; ++i)
			{
				delete[] mPhotonSpheres[i].Index;
				delete[] mPhotonSpheres[i].Distances2;
			}
			delete[] mPhotonSpheres;
		}
	}

	constexpr float NormalOffset = 0.0001f;
	void PhotonIntegrator::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);

		if (mMap)
		{
			delete mMap;

			for (uint32 i = 0; i < mSphereCount; ++i)
			{
				delete[] mPhotonSpheres[i].Index;
				delete[] mPhotonSpheres[i].Distances2;
			}
			delete[] mPhotonSpheres;
		}


		mMap = new Photon::PhotonMap(renderer->settings().maxPhotons());

		// We should sample lights differently... not like this.
		auto lightList = renderer->lights();
		const size_t sampleSize = renderer->settings().maxPhotons() / lightList.size();
		StratifiedSampler sampler(renderer->random(), 200);
		for (RenderEntity* light : lightList)
		{
			uint32 photonsShoot = 0;
			for (size_t i = 0; i < sampleSize; ++i)
			{
				FacePoint lightSample = light->getRandomFacePoint(sampler, renderer->random());
				lightSample.setNormal(
					Projection::align(lightSample.normal(),
						Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat())));
				
				Ray ray(PM::pm_Add(lightSample.vertex(), PM::pm_Scale(lightSample.normal(), NormalOffset)),
					lightSample.normal(), 1);// Depth will not be incremented, but we use one to hack non-camera objects in. 
				ray.setSpectrum(light->material()->applyEmission(lightSample, lightSample.normal()));

				for (uint32 j = 0; j < renderer->settings().maxRayDepth(); ++j)
				{
					if (ray.spectrum().isOnlyZero())
						break;

					FacePoint collision;
					RenderEntity* entity = renderer->shoot(ray, collision, nullptr, nullptr);

					if (entity && entity->material())
					{
						Spectrum diff;
						Spectrum spec;

						// Russian Roulette
						float rnd = renderer->random().getFloat();
						if (rnd < entity->material()->roughness())// Diffuse
						{
							// Always store when diffuse
							mMap->store(ray.spectrum(), collision.vertex(), ray.direction());
							photonsShoot++;

							rnd = renderer->random().getFloat();
							if (rnd < entity->material()->roughness())// Shoot
							{
								PM::vec3 newDir = PM::pm_SetW(Projection::align(collision.normal(),
									Projection::cos_hemi(renderer->random().getFloat(), renderer->random().getFloat())), 0);

								entity->material()->apply(collision, newDir, ray.direction(), ray.spectrum(), diff, spec);
								ray.setSpectrum(diff + spec);
								ray.setDirection(newDir);
								ray.setStartPosition(PM::pm_Add(collision.vertex(), PM::pm_Scale(newDir, NormalOffset)));
							}
							else
							{
								break;// Absorb
							}
						}
						else// Reflect
						{
							PM::vec3 traV;
							PM::vec3 reflV;

							float reflWeight = entity->material()->emitReflectionVector(collision, ray.direction(), reflV);
							float traWeight = entity->material()->emitTransmissionVector(collision, ray.direction(), traV);

							rnd = renderer->random().getFloat();

							if (rnd < reflWeight)
							{
								entity->material()->apply(collision, reflV, ray.direction(), ray.spectrum(), diff, spec);
								ray.setSpectrum(spec);
								ray.setDirection(reflV);
								ray.setStartPosition(PM::pm_Add(collision.vertex(), PM::pm_Scale(reflV, NormalOffset)));
							}
							else if (rnd < reflWeight + traWeight)
							{
								entity->material()->apply(collision, traV, ray.direction(), ray.spectrum(), diff, spec);
								ray.setSpectrum(spec);
								ray.setDirection(traV);
								ray.setStartPosition(PM::pm_Add(collision.vertex(), PM::pm_Scale(traV, NormalOffset)));
							}
							else //Ignore?
							{
								break;
							}
						}
					}
					else
					{
						break;
					}
				}
			}

			if (photonsShoot != 0)
				mMap->scalePhotonPower(1.0f/photonsShoot);
		}

		mMap->balanceTree();

		mSphereCount = renderer->threads();
		mPhotonSpheres = new Photon::PhotonSphere[renderer->threads()];
		for (uint32 i = 0; i < renderer->threads(); ++i)
		{
			mPhotonSpheres[i].Distances2 = new float[renderer->settings().maxPhotonGatherCount() + 1];
			mPhotonSpheres[i].Index = new const Photon::Photon*[renderer->settings().maxPhotonGatherCount() + 1];
			mPhotonSpheres[i].Max = renderer->settings().maxPhotonGatherCount();
		}
	}

	constexpr float K = 1.1;

	Spectrum PhotonIntegrator::apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context)
	{
		if (mMap->isEmpty())
			return Spectrum();

		Photon::PhotonSphere* sphere = &mPhotonSpheres[context->threadNumber()];
		
		sphere->Found = 0;
		sphere->Center = point.vertex();
		sphere->GotHeap = false;
		sphere->Distances2[0] =
			context->renderer()->settings().maxPhotonGatherRadius() * context->renderer()->settings().maxPhotonGatherRadius();

		mMap->locate(*sphere, 1);

		Spectrum full;
		if (sphere->Found >= 8)
		{
			for (uint64 i = 1; i <= sphere->Found; ++i)
			{
				Spectrum spec;
				Spectrum diff;

				const Photon::Photon* photon = sphere->Index[i];
				PM::vec3 pos = PM::pm_Set(photon->Position[0], photon->Position[1], photon->Position[2], 1);
				PM::vec3 dir = mMap->photonDirection(photon);

				float d = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(point.vertex(), pos));
				float w = 1 - d / (K*sphere->Distances2[0]);

				entity->material()->apply(point, in.direction(), PM::pm_Negate(dir), Spectrum(photon->Power), diff, spec);

				full += (spec + diff)*w;
			}
		}

		return (full) * (PM_INV_PI_F / ((1 - 2/(3*K)) * sphere->Distances2[0]));
	}
}