#include "PPMIntegrator.h"

#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"

#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"

#include "photon/Photon.h"
#include "photon/PhotonMap.h"

#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderThread.h"

#include "spectral/RGBConverter.h"

#include "Logger.h"

namespace PR
{
	PPMIntegrator::PPMIntegrator() :
		Integrator(), mRenderer(nullptr), mPhotonMap(nullptr), mPhotonSpheres(nullptr), mCurrentPassRadius2(1)
	{
	}

	PPMIntegrator::~PPMIntegrator()
	{
		if(mPhotonMap)
			delete mPhotonMap;
		
		if(mPhotonSpheres)
		{
			PR_ASSERT(mRenderer);

			for (uint32 i = 0; i < mRenderer->threads(); ++i)
			{
				delete[] mPhotonSpheres[i].Index;
				delete[] mPhotonSpheres[i].Distances2;
			}
			delete[] mPhotonSpheres;
		}

		for(Light* l : mLights)
			delete l;

		mLights.clear();
	}

	void PPMIntegrator::init(Renderer* renderer)
	{
		PR_ASSERT(!mPhotonMap);
		PR_ASSERT(!mPhotonSpheres);
		PR_ASSERT(mLights.empty());
		PR_ASSERT(renderer);

		PR_ASSERT(renderer->settings().ppm().maxPhotonsPerPass() > 0);
		PR_ASSERT(renderer->settings().ppm().maxGatherCount() > 0);
		PR_ASSERT(renderer->settings().ppm().maxGatherRadius() > PM_EPSILON);

		mRenderer = renderer;
		mPhotonMap = new Photon::PhotonMap(renderer->settings().ppm().maxPhotonsPerPass());

		mPhotonSpheres = new Photon::PhotonSphere[mRenderer->threads()];
		for (uint32 i = 0; i < mRenderer->threads(); ++i)
		{
			mPhotonSpheres[i].Distances2 = new float[renderer->settings().ppm().maxGatherCount() + 1];
			mPhotonSpheres[i].Index = new const Photon::Photon*[renderer->settings().ppm().maxGatherCount() + 1];
			mPhotonSpheres[i].Max = renderer->settings().ppm().maxGatherCount();
		}
		
		mCurrentPassRadius2 = renderer->settings().ppm().maxGatherRadius() * renderer->settings().ppm().maxGatherRadius();

		// Assign photons to each light
		constexpr uint64 MinPhotons = 10;
		const std::list<RenderEntity*>& lightList = mRenderer->lights();

		const uint64 k = MinPhotons * lightList.size();
		if (k >= renderer->settings().ppm().maxPhotonsPerPass()) // Not enough photons given.
		{
			PR_LOGGER.logf(L_Warning, M_Integrator, "Not enough photons per pass given. At least %llu is good.", k);

			for(RenderEntity* light : lightList)
				mLights.push_back(new Light({light, MinPhotons, light->surfaceArea(nullptr)}));
		}
		else
		{
			const uint64 d = renderer->settings().ppm().maxPhotonsPerPass() - k;

			float fullArea = 0;
			for(RenderEntity* light : lightList)
				fullArea += light->surfaceArea(nullptr);

			for(RenderEntity* light : lightList)
			{
				const float surface = light->surfaceArea(nullptr);
				mLights.push_back(new Light({light,
					MinPhotons + (uint64)std::ceil(d * (surface / fullArea)),
					surface}));
			}
		}
	}

	void PPMIntegrator::onStart()
	{
	}

	constexpr float A = 0.5;
	constexpr float K = 1.1;
	
	void PPMIntegrator::onNextPass(uint32 pass)
	{
		PR_LOGGER.logf(L_Info, M_Integrator, "Preparing PPM pass %i", pass + 1);

		// Recalculate radius
		if(pass > 0)
			mCurrentPassRadius2 *= (pass + A) / (float)(pass + 1);

#ifdef PR_DEBUG
		PR_LOGGER.logf(L_Debug, M_Integrator, "  -> Radius2: %f", mCurrentPassRadius2);
#endif

		// Emit all lights
		mPhotonMap->reset();

		Random random;
		for (Light* light : mLights)
		{
			const size_t sampleSize = light->Photons;
			MultiJitteredSampler sampler(random, sampleSize);

			size_t photonsShoot = 0;
			for (size_t i = 0; i < sampleSize*4 && photonsShoot < sampleSize; ++i)
			{
				float full_pdf;
				FaceSample lightSample = light->Entity->getRandomFacePoint(sampler,(uint32) i, full_pdf);
				lightSample.Ng = Projection::align(lightSample.Ng,
						Projection::cos_hemi(random.getFloat(), random.getFloat(), full_pdf));
				
				Ray ray = Ray::safe(lightSample.P, lightSample.Ng, 1);

				Spectrum flux;
				if(lightSample.Material->emission())
				{
					ShaderClosure lsc;
					lsc.P = lightSample.P;
					lsc.Ng = lightSample.Ng;
					lsc.N = lightSample.Ng;
					lsc.Nx = lightSample.Nx;
					lsc.Ny = lightSample.Ny;
					lsc.UV = lightSample.UV;
					lsc.Material = lightSample.Material;
					flux = lightSample.Material->emission()->eval(lsc);
				}

				uint32 diffuseBounces = 0;
				for (uint32 j = 0; j < mRenderer->settings().maxRayDepth(); ++j)
				{
					ShaderClosure sc;
					RenderEntity* entity = mRenderer->shoot(ray, sc, nullptr);

					if (entity && sc.Material && sc.Material->canBeShaded())
					{
						const float NdotL = PM::pm_MaxT(0.0f, -PM::pm_Dot3D(sc.N, ray.direction()));
						PM::vec3 nextDir;

						if (NdotL <= PM_EPSILON)
							break;

						float pdf;
						PM::vec3 s = PM::pm_Set(random.getFloat(),
							random.getFloat(),
							random.getFloat());
						nextDir = sc.Material->sample(sc, s, pdf);

						if (pdf > PM_EPSILON && !std::isinf(pdf))// Diffuse
						{
							// Always store when diffuse
							mPhotonMap->store(flux, sc.P, ray.direction(), pdf);
							photonsShoot++;

							diffuseBounces++;

							if (diffuseBounces > mRenderer->settings().maxDiffuseBounces())// Shoot
								break;// Absorb
						}
						else if(!std::isinf(pdf))// Absorb
						{
							break;
						}

						
						MSI::balance(flux, full_pdf,
							sc.Material->apply(sc, nextDir) * NdotL, pdf);

						ray = ray.next(sc.P, nextDir);
						ray.setDepth(1);
					}
					else // Nothing found, abort
					{
						break;
					}
				}
			}

			//if (photonsShoot != 0)
			//  	mPhotonMap->scalePhotonPower(1.0f/photonsShoot);
			//if (photonsShoot != 0)
			//  	mPhotonMap->scalePhotonPower(light->Surface);

#ifdef PR_DEBUG
			PR_LOGGER.logf(L_Debug, M_Integrator, "    -> Per Light Samples: %llu / %llu [%3.2f%]",
				sampleSize, mRenderer->settings().ppm().maxPhotonsPerPass(),
				100 * sampleSize / (double)mRenderer->settings().ppm().maxPhotonsPerPass());
			PR_LOGGER.logf(L_Debug, M_Integrator, "      -> Shoot: %llu / %llu [%3.2f%]",
				photonsShoot, mRenderer->settings().ppm().maxPhotonsPerPass(),
				100 * photonsShoot / (double)mRenderer->settings().ppm().maxPhotonsPerPass());
#endif
		}

		mPhotonMap->balanceTree();
		
#ifdef PR_DEBUG
		PR_LOGGER.logf(L_Debug, M_Integrator, "  -> Photons: %llu / %llu [%3.2f%]",
			mPhotonMap->storedPhotons(), mRenderer->settings().ppm().maxPhotonsPerPass(),
			100 * mPhotonMap->storedPhotons() / (double)mRenderer->settings().ppm().maxPhotonsPerPass());
#endif
	}

	void PPMIntegrator::onEnd()
	{
	}

	bool PPMIntegrator::needNextPass(uint32 pass) const
	{
		return pass < mRenderer->settings().ppm().maxPassCount();
	}

	void PPMIntegrator::onThreadStart(RenderContext* context)
	{
	}

	void PPMIntegrator::onPrePass(RenderContext* context, uint32 i)
	{
	}

	void PPMIntegrator::onPass(RenderTile* tile, RenderContext* context, uint32 pass)
	{
		for (uint32 y = tile->sy(); y < tile->ey() && !context->thread()->shouldStop(); ++y)
		{
			for (uint32 x = tile->sx(); x < tile->ex() && !context->thread()->shouldStop(); ++x)
			{
				context->render(x, y, tile->samplesRendered(), pass);
			}
		}
	}

	void PPMIntegrator::onPostPass(RenderContext* context, uint32 i)
	{
	}

	void PPMIntegrator::onThreadEnd(RenderContext* context)
	{
	}

	uint64 PPMIntegrator::maxSamples(const Renderer* renderer) const
	{
		return renderer->renderWidth() * renderer->renderHeight() *
			renderer->settings().maxPixelSampleCount() * renderer->settings().ppm().maxPassCount();
	}

	Spectrum PPMIntegrator::apply(const Ray& in, RenderContext* context, uint32 pass)
	{
		PR_ASSERT(mPhotonMap);

		ShaderClosure sc;
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		if (!entity || !sc.Material)
			return applied;
		
		return applied + applyRay(in, sc, context, pass);
	}

	Spectrum PPMIntegrator::applyRay(const Ray& in, const ShaderClosure& sc, RenderContext* context, uint32 pass)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Hemisphere sampling
		RandomSampler hemiSampler(context->random());
		for (uint32 i = 0;
			i < context->renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
			++i)
		{
			float pdf;
			Spectrum weight;
			PM::vec3 rnd = hemiSampler.generate3D(i);
			PM::vec3 dir = sc.Material->sample(sc, rnd, pdf);
			const float NdotL = PM::pm_MaxT(0.0f, -PM::pm_Dot3D(dir, sc.N));

			if (NdotL > PM_EPSILON && pdf > PM_EPSILON)
			{
				Ray ray = in.next(sc.P, dir);

				ShaderClosure sc2;
				Spectrum applied;
				if (context->shootWithEmission(applied, ray, sc2) && sc2.Material && std::isinf(pdf))
					applied += applyRay(ray, sc2, context, pass);

				weight = sc.Material->apply(sc, ray.direction()) * applied * NdotL;
			}

			MSI::power(full_weight, full_pdf, weight, pdf);
		}

		if (!std::isinf(full_pdf))
		{
			float inf_pdf;
			Spectrum inf_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::balance(full_weight, full_pdf, inf_weight, inf_pdf);
		}

		if (!std::isinf(full_pdf) && !mPhotonMap->isEmpty())
		{
			Photon::PhotonSphere* sphere = &mPhotonSpheres[context->threadNumber()];
			
			sphere->Found = 0;
			sphere->Center = sc.P;
			sphere->Normal = sc.N;
			sphere->SqueezeWeight =
				context->renderer()->settings().ppm().squeezeWeight() * context->renderer()->settings().ppm().squeezeWeight();
			sphere->GotHeap = false;
			sphere->Distances2[0] = mCurrentPassRadius2;

			switch (context->renderer()->settings().ppm().gatheringMode())
			{
			default:
			case PGM_Sphere:
				mPhotonMap->locateSphere(*sphere, 1);
				break;
			case PGM_Dome:
				mPhotonMap->locateDome(*sphere, 1);
				break;
			}

			if (sphere->Found >= 1)
			{
				//PR_LOGGER.logf(L_Debug, M_Integrator, "Found %i", sphere->Found);
				for (uint64 i = 1; i <= sphere->Found; ++i)
				{
					const Photon::Photon* photon = sphere->Index[i];
					const PM::vec3 dir = mPhotonMap->photonDirection(photon);

					//const PM::vec3 pos = PM::pm_Set(photon->Position[0], photon->Position[1], photon->Position[2], 1);

					const float d = std::sqrt(sphere->Distances2[i]/sphere->Distances2[0]);
					const float w = 1 - d / K;

					Spectrum weight;
					if (w > PM_EPSILON)
					{
	#ifdef PR_USE_PHOTON_RGB
						weight = sc.Material->apply(sc, dir) *
							RGBConverter::toSpec(photon->Power[0], photon->Power[1], photon->Power[2])*w;
	#else
						weight = sc.Material->apply(sc, dir) * Spectrum(photon->Power)*w;
	#endif
					}

					MSI::power(full_weight, full_pdf, weight * (PM_INV_PI_F / ((1 - 2/(3*K)) * sphere->Distances2[0])), photon->PDF);
				}
			}
		}

		return full_weight;
	}
}