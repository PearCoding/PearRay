#include "PhotonAffector.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "material/Material.h"

#include "sampler/MultiJitteredSampler.h"
#include "math/Projection.h"

#include "photon/PhotonMap.h"

#ifdef PR_USE_PHOTON_RGB
# include "spectral/RGBConverter.h"
#endif

namespace PR
{
	PhotonAffector::PhotonAffector() :
		Affector(), mMap(nullptr), mPhotonSpheres(nullptr), mSphereCount(0)
	{
	}

	PhotonAffector::~PhotonAffector()
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

	constexpr float ScaleFactor = 1.0f;// How much scale should be used for lights.
	void PhotonAffector::init(Renderer* renderer)
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

		if (!mMap)
			return;

		Random random;
		// We should sample lights differently... not like this.
		auto lightList = renderer->lights();
		for (RenderEntity* light : lightList)
		{
			const size_t sampleSize = renderer->settings().maxPhotons() / lightList.size();
			MultiJitteredSampler sampler(random, (uint32)sampleSize);

			size_t photonsShoot = 0;
			for (size_t i = 0; i < sampleSize*4 && photonsShoot < sampleSize; ++i)
			{
				FacePoint lightSample = light->getRandomFacePoint(sampler,(uint32) i % sampleSize);
				lightSample.setNormal(
					Projection::align(lightSample.normal(),
						Projection::cos_hemi(random.getFloat(), random.getFloat())));
				
				Ray ray(lightSample.vertex(), lightSample.normal(), 1);// Depth will not be incremented, but we use one to hack non-camera objects into the scene. 

				Spectrum flux = lightSample.material()->applyEmission(lightSample, lightSample.normal());

				uint32 diffuseBounces = 0;
				uint32 specBounces = 0;
				for (uint32 j = 0; j < renderer->settings().maxRayDepth(); ++j)
				{
					/*if (flux.isOnlyZero())
						break;*/

					FacePoint collision;
					RenderEntity* entity = renderer->shoot(ray, collision, nullptr, nullptr);

					if (entity && collision.material() && collision.material()->canBeShaded())
					{
						const float NdotL = std::abs(PM::pm_Dot3D(collision.normal(), ray.direction()));
						PM::vec3 nextDir;

						if (NdotL < PM_EPSILON)
							break;

						float pdf;
						PM::vec3 s = PM::pm_Set(random.getFloat(),
							random.getFloat(),
							random.getFloat());
						nextDir = collision.material()->sample(collision, s, ray.direction(), pdf);

						if (pdf > PM_EPSILON && !std::isinf(pdf))// Diffuse
						{
							if (specBounces >= renderer->settings().minPhotonSpecularBounces())// Never calculate direct lightning
							{
								// Always store when diffuse
								mMap->store(flux, collision.vertex(), ray.direction());
								photonsShoot++;
							}

							diffuseBounces++;

							if (diffuseBounces > renderer->settings().maxPhotonDiffuseBounces())// Shoot
							{
								break;// Absorb
							}
						}
						else if(pdf > PM_EPSILON)// Reflect
						{
							specBounces++;
						}
						else
						{
							break;
						}

						flux *= collision.material()->apply(collision, nextDir, ray.direction()) * 
							(NdotL / (std::isinf(pdf) ? 1 : pdf));
						ray.setDirection(nextDir);
						ray.setStartPosition(collision.vertex());
					}
					else // Nothing found, abort
					{
						break;
					}
				}
			}

			if (photonsShoot != 0)
				mMap->scalePhotonPower(ScaleFactor/photonsShoot);
		}

		mMap->balanceTree();

		mSphereCount = renderer->threads();
		mPhotonSpheres = new Photon::PhotonSphere[mSphereCount];
		for (uint32 i = 0; i < mSphereCount; ++i)
		{
			mPhotonSpheres[i].Distances2 = new float[renderer->settings().maxPhotonGatherCount() + 1];
			mPhotonSpheres[i].Index = new const Photon::Photon*[renderer->settings().maxPhotonGatherCount() + 1];
			mPhotonSpheres[i].Max = renderer->settings().maxPhotonGatherCount();
		}
	}

	constexpr float K = 1.1;
	Spectrum PhotonAffector::apply(const Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context)
	{
		if (!mMap || mMap->isEmpty())
			return Spectrum();

		Photon::PhotonSphere* sphere = &mPhotonSpheres[context->threadNumber()];
		
		sphere->Found = 0;
		sphere->Center = point.vertex();
		sphere->Normal = point.normal();
		sphere->SqueezeWeight =
			context->renderer()->settings().photonSqueezeWeight() * context->renderer()->settings().photonSqueezeWeight();
		sphere->GotHeap = false;
		sphere->Distances2[0] =
			context->renderer()->settings().maxPhotonGatherRadius() * context->renderer()->settings().maxPhotonGatherRadius();

		switch (context->renderer()->settings().photonGatheringMode())
		{
		default:
		case PGM_Sphere:
			mMap->locateSphere(*sphere, 1);
			break;
		case PGM_Dome:
			mMap->locateDome(*sphere, 1);
			break;
		}

		Spectrum full;
		if (sphere->Found >= 1)
		{
			for (uint64 i = 1; i <= sphere->Found; ++i)
			{
				const Photon::Photon* photon = sphere->Index[i];
				const PM::vec3 dir = mMap->photonDirection(photon);

				const PM::vec3 pos = PM::pm_Set(photon->Position[0], photon->Position[1], photon->Position[2], 1);

				const float d = std::sqrt(sphere->Distances2[i]/sphere->Distances2[0]);
				const float w = 1 - d / K;

				if (w >= PM_EPSILON)
				{
#ifdef PR_USE_PHOTON_RGB
					full += point.material()->apply(point, in.direction(), dir) *
						RGBConverter::toSpec(photon->Power[0], photon->Power[1], photon->Power[2])*w;
#else
					full += point.material()->apply(point, in.direction(), dir) * Spectrum(photon->Power)*w;
#endif
				}
			}
		}

		return full * (PM_INV_PI_F / ((1 - 2/(3*K)) * sphere->Distances2[0]));
	}
}