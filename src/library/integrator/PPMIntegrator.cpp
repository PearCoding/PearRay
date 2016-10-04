#include "PPMIntegrator.h"

#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"
#include "math/SphereMap.h"

#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"

#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderThread.h"

#include "spectral/RGBConverter.h"

#include "Logger.h"

//#define PR_PPM_CONE_FILTER

namespace PR
{
	constexpr uint32 MAX_THETA_SIZE = 128;
	constexpr uint32 MAX_PHI_SIZE = MAX_THETA_SIZE*2;

	PPMIntegrator::PPMIntegrator() :
		Integrator(), mRenderer(nullptr),
		mPhotonMap(nullptr), mThreadData(nullptr),
		mProjMaxTheta(MAX_THETA_SIZE), mProjMaxPhi(MAX_PHI_SIZE)
	{
	}

	PPMIntegrator::~PPMIntegrator()
	{
		if(mPhotonMap)
			delete mPhotonMap;
		
		if(mThreadData)
		{
			PR_ASSERT(mRenderer);

			for (uint32 i = 0; i < mRenderer->threads(); ++i)
			{
				delete[] mThreadData[i].PhotonSearchSphere.Index;
				delete[] mThreadData[i].PhotonSearchSphere.Distances2;

			}
			delete[] mThreadData;
		}

		for(Light* l : mLights)
		{
			if(l->Proj)
				delete l->Proj;
			
			delete l;
		}

		mLights.clear();
	}

	void PPMIntegrator::init(Renderer* renderer)
	{
		PR_ASSERT(!mPhotonMap);
		PR_ASSERT(!mThreadData);
		PR_ASSERT(mLights.empty());
		PR_ASSERT(renderer);

		PR_ASSERT(renderer->settings().ppm().maxPhotonsPerPass() > 0);
		PR_ASSERT(renderer->settings().ppm().maxGatherCount() > 0);
		PR_ASSERT(renderer->settings().ppm().maxGatherRadius() > PM_EPSILON);

		mRenderer = renderer;
		mPhotonMap = new Photon::PointMap<Photon::Photon>(renderer->settings().ppm().maxPhotonsPerPass());

		mThreadData = new ThreadData[mRenderer->threads()];
		for (uint32 i = 0; i < mRenderer->threads(); ++i)
		{
			mThreadData[i].PhotonSearchSphere.Distances2 = new float[renderer->settings().ppm().maxGatherCount() + 1];
			mThreadData[i].PhotonSearchSphere.Index = new const Photon::Photon*[renderer->settings().ppm().maxGatherCount() + 1];
			mThreadData[i].PhotonSearchSphere.Max = renderer->settings().ppm().maxGatherCount();
			mThreadData[i].PhotonSearchSphere.SqueezeWeight =
				renderer->settings().ppm().squeezeWeight() *
				renderer->settings().ppm().squeezeWeight();
		}

		// Assign photon count to each light
		mProjMaxTheta = PM::pm_MaxT<uint32>(8, MAX_THETA_SIZE*renderer->settings().ppm().projectionMapQuality());
		mProjMaxPhi = PM::pm_MaxT<uint32>(8, MAX_PHI_SIZE*renderer->settings().ppm().projectionMapQuality());

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
				SphereMap* map = nullptr;
				if(renderer->settings().ppm().projectionMapWeight() > PM_EPSILON)
					map = new SphereMap(mProjMaxTheta, mProjMaxPhi);
				
				const float surface = light->surfaceArea(nullptr);
				auto l = new Light({light,
					MinPhotons + (uint64)std::ceil(d * (surface / fullArea)),
					surface,
					map});
				mLights.push_back(l);

#ifdef PR_DEBUG
				PR_LOGGER.logf(L_Debug, M_Integrator, "PPM Light %s %llu photons %f m2",
					light->name().c_str(), l->Photons, l->Surface);
#endif
			}
		}
	}

	/* When using projection maps, never set something to 0 and disable it that way.
	 * It will get biased otherwise. Setting it to a very low value ensures consistency and convergency.
	 */
	constexpr float SAFE_DISTANCE = 1;
	void PPMIntegrator::onStart()
	{
		// Setup Projection Maps
		if(mRenderer->settings().ppm().projectionMapWeight() <= PM_EPSILON)
			return;

		PR_LOGGER.logf(L_Info, M_Integrator, "Calculating Projection Maps (%i,%i)", mProjMaxTheta, mProjMaxPhi);

		const float DELTA_W = mRenderer->settings().ppm().projectionMapWeight();
		const float CAUSTIC_PREF = mRenderer->settings().ppm().projectionMapPreferCaustic();

		Random random;
		for(Light* l : mLights)
		{
			if(!l->Proj)
				continue;
	
			const Sphere outerSphere = l->Entity->worldBoundingBox().outerSphere();
			
			for(uint32 thetaI = 0; thetaI < mProjMaxTheta; ++thetaI)
			{
				const float theta = PM_PI_F * thetaI / (float)mProjMaxTheta;
				float thCos, thSin;
				PM::pm_SinCosT(theta, thSin, thCos);

				for(uint32 phiI = 0; phiI < mProjMaxPhi; ++phiI)
				{
					const float phi = PM_2_PI_F * phiI / (float)mProjMaxPhi;
					float phCos, phSin;
					PM::pm_SinCosT(phi, phSin, phCos);
					PM::vec3 dir = PM::pm_Set(thSin * phCos, thSin * phSin, thCos);

					Ray ray(PM::pm_Zero(), 
						PM::pm_Add(outerSphere.position(), PM::pm_Scale(dir, outerSphere.radius() + SAFE_DISTANCE)),
						PM::pm_Negate(dir));
					FaceSample lightSample;
					float weight = 0;
					if(l->Entity->checkCollision(ray, lightSample))
					{
						ray = Ray::safe(PM::pm_Zero(), lightSample.P, dir, 0, 0, RF_FromLight);
						float pdf = std::abs(PM::pm_Dot3D(dir, lightSample.Ng));
						
						uint32 j;// Calculates specular count
						for (j = 0;
							j < mRenderer->settings().maxRayDepth();
							++j)
						{
							ShaderClosure sc;
							RenderEntity* entity = mRenderer->shoot(ray, sc, nullptr);

							if (entity && sc.Material && sc.Material->canBeShaded())
							{							
								PM::vec3 nextDir;
								float l_pdf;

								PM::vec3 s = PM::pm_Set(random.getFloat(),
									random.getFloat(),
									random.getFloat());
								nextDir = sc.Material->sample(sc, s, l_pdf);

								const float NdotL = std::abs(PM::pm_Dot3D(nextDir, sc.N));
								if(pdf*NdotL <= PM_EPSILON)// Drop this one.
								{
									if(j == 0)
										pdf = 0;
									else
										j--;
									
									break;
								}

								pdf *= NdotL;

								if (!std::isinf(l_pdf))// Diffuse
								{
									pdf *= l_pdf;
									break;
								}
								
								ray = ray.next(sc.P, nextDir);
							}
							else // Nothing found, abort
							{
								pdf = 0;
								break;
							}
						}
						weight = pdf*(1+(j/(float)mRenderer->settings().maxRayDepth())*CAUSTIC_PREF);
					}
					
					l->Proj->setProbabilityWithIndex(thetaI, phiI, (weight+0.01f) * DELTA_W + (1-DELTA_W));
				}
			}

			l->Proj->setup();
		}
	}

	constexpr float K = 1.1;// Cone filter
	void PPMIntegrator::onNextPass(uint32 pass, bool& clean)
	{
		clean = true;// Clear sample, error, etc information prior next pass.
		PR_LOGGER.logf(L_Info, M_Integrator, "Preparing PPM pass %i", pass + 1);

		if (pass == 0)// First pass is not handled here
			return;

		// Emit all lights
		mPhotonMap->reset();

		Random random;
		for (Light* light : mLights)
		{
			const Sphere outerSphere = light->Entity->worldBoundingBox().outerSphere();

			const size_t sampleSize = light->Photons;
			MultiJitteredSampler sampler(random, sampleSize);

			size_t photonsShoot = 0;
			for (size_t i = 0;
				 i < sampleSize*(mRenderer->settings().maxDiffuseBounces()+1) &&
				 	photonsShoot < sampleSize;
				 ++i)
			{
				float t_pdf = 0;
				FaceSample lightSample;
				PM::vec3 dir;
				
				if(light->Proj)
				{
					dir = light->Proj->sample(random.getFloat(), random.getFloat(), random.getFloat(), random.getFloat(), t_pdf);
					
					Ray ray(PM::pm_Zero(),
							PM::pm_Add(outerSphere.position(), PM::pm_Scale(dir, outerSphere.radius() + SAFE_DISTANCE)),
						PM::pm_Negate(dir));
					if(!light->Entity->checkCollision(ray, lightSample))
						continue;
				}
				else
				{
					lightSample = light->Entity->getRandomFacePoint(sampler,(uint32) i, t_pdf);

					float t_pdf2 = 0;
					dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
								Projection::cos_hemi(random.getFloat(), random.getFloat(), t_pdf2));
					t_pdf *= t_pdf2;
				}
				
				Ray ray = Ray::safe(PM::pm_Zero(), lightSample.P, dir, 0, 0, RF_FromLight);

				Spectrum radiance;
				if(lightSample.Material->emission())
				{
					ShaderClosure lsc = lightSample;
					radiance = lightSample.Material->emission()->eval(lsc);
					//radiance /= t_pdf;
				}
				else
				{
					continue;
				}

				uint32 diffuseBounces = 0;
				for (uint32 j = 0;
					 j < mRenderer->settings().maxRayDepth();
					 ++j)
				{
					ShaderClosure sc;
					RenderEntity* entity = mRenderer->shoot(ray, sc, nullptr);

					if (entity && sc.Material && sc.Material->canBeShaded() &&
						sc.NdotV > PM_EPSILON)
					{
						PM::vec3 nextDir;
						float pdf;

						PM::vec3 s = PM::pm_Set(random.getFloat(),
							random.getFloat(),
							random.getFloat());
						nextDir = sc.Material->sample(sc, s, pdf);

						if (pdf > PM_EPSILON && !std::isinf(pdf))// Diffuse
						{
							// Always store when diffuse
							Photon::Photon photon;
							PM::pm_Store3D(sc.P, photon.Position);
							mPhotonMap->mapDirection(PM::pm_Negate(ray.direction()), photon.Theta, photon.Phi);
#ifdef PR_USE_PHOTON_RGB
							RGBConverter::convert(radiance, photon.Power[0], photon.Power[1], photon.Power[2]);
#else
							for(int i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
								photon.Power[i] = radiance.value(i);
#endif
							mPhotonMap->store(sc.P, photon);
							photonsShoot++;

							diffuseBounces++;

							if (diffuseBounces > mRenderer->settings().maxDiffuseBounces())
								break;// Absorb
						}
						else if(!std::isinf(pdf))// Absorb
						{
							break;
						}
						
						radiance *= sc.Material->eval(sc, nextDir, sc.NdotV) * sc.NdotV;
						ray = ray.next(sc.P, nextDir);
					}
					else // Nothing found, abort
					{
						break;
					}
				}
			}

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
		return pass < mRenderer->settings().ppm().maxPassCount() + 1;
	}

	void PPMIntegrator::onThreadStart(RenderContext* context)
	{
	}

	void PPMIntegrator::onPrePass(RenderContext* context, uint32 pass)
	{
		if(pass > 0)
		{
#ifdef PR_DEBUG
			PR_LOGGER.log(L_Debug, M_Integrator, "PrePass: Accumulating Photons");
#endif

			const float A = 1 - context->renderer()->settings().ppm().contractRatio();

			Photon::PointSphere<Photon::Photon>& sphere = mThreadData[context->threadNumber()].PhotonSearchSphere;

			for(RayHitPoint& p : mThreadData[context->threadNumber()].HitPoints)
			{
				Spectrum full_weight;

				sphere.Found = 0;
				sphere.Center = p.SC.P;
				sphere.Normal = p.SC.N;
				sphere.GotHeap = false;
				sphere.Distances2[0] = p.CurrentRadius;

				switch (context->renderer()->settings().ppm().gatheringMode())
				{
				default:
				case PGM_Sphere:
					mPhotonMap->locateSphere(sphere, 1);
					break;
				case PGM_Dome:
					mPhotonMap->locateDome(sphere, 1);
					break;
				}

				if (sphere.Found >= 4)
				{
					//PR_LOGGER.logf(L_Debug, M_Integrator, "Found %i", sphere->Found);
					for (uint64 i = 1; i <= sphere.Found; ++i)
					{
						const Photon::Photon* photon = sphere.Index[i];
						const PM::vec3 dir = mPhotonMap->evalDirection(photon->Theta, photon->Phi);

						Spectrum weight;

	#ifdef PR_PPM_CONE_FILTER
						const float d = sphere.Distances2[i]/sphere.Distances2[0];
						const float w = 1 - d / K;

						if (w > PM_EPSILON)
						{
	#endif//PR_PPM_CONE_FILTER
							const float NdotL = std::abs(PM::pm_Dot3D(p.SC.N, dir));
	#ifdef PR_USE_PHOTON_RGB
							weight = p.SC.Material->eval(p.SC, dir, NdotL) *
								RGBConverter::toSpec(photon->Power[0], photon->Power[1], photon->Power[2]);
	#else
							weight = p.SC.Material->eval(p.SC, dir, NdotL) * Spectrum(photon->Power);
	#endif//PR_USE_PHOTON_RGB

	#ifdef PR_PPM_CONE_FILTER 
						}
						full_weight += weight * (w * (PM_INV_PI_F / ((1 - 2/(3*K)) * sphere.Distances2[0])));
	#else
						full_weight += weight * (PM_INV_PI_F / sphere.Distances2[0]);
	#endif//PR_PPM_CONE_FILTER
					}
					
					// Change radius, photons etc.
					const uint64 newN = p.CurrentPhotons + std::ceil(A*sphere.Found);
					const float fraction = newN / (float)(p.CurrentPhotons + sphere.Found);
					p.CurrentRadius *= fraction;
					p.CurrentPhotons = newN;
					p.CurrentFlux = (p.CurrentFlux + full_weight) * fraction;
				}

				const float inv = 1.0f / (context->renderer()->settings().ppm().maxPhotonsPerPass() * pass);

				// Render result:
				context->renderer()->pushPixel_Normalized(p.Weight * p.CurrentFlux * inv,
					p.PixelX, p.PixelY);
			}
		}
	}

	void PPMIntegrator::onPass(RenderTile* tile, RenderContext* context, uint32 pass)
	{
		// TODO: Find a solution for the background.
		if(pass > 0)// Do we need other passes? Except for background.
			return;

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
			renderer->settings().maxPixelSampleCount() * (renderer->settings().ppm().maxPassCount() + 1);
	}

	uint64 PPMIntegrator::maxPasses(const Renderer* renderer) const
	{
		return renderer->settings().ppm().maxPassCount() + 1;
	}

	Spectrum PPMIntegrator::apply(const Ray& in, RenderContext* context, uint32 pass)
	{
		ShaderClosure sc;
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		if (!entity || !sc.Material)
			return applied;
		
		if(pass == 0)
			return applied + firstPass(Spectrum(1), in, sc, context);
		else
			return applied + otherPass(in, sc, context);
	}

	Spectrum PPMIntegrator::firstPass(const Spectrum& weight, const Ray& in, const ShaderClosure& sc, RenderContext* context)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float pdf;
		PM::vec3 dir = sc.Material->sample(sc, PM::pm_Zero(), pdf);

		Spectrum full_weight;
		if(std::isinf(pdf))// Specular
		{
			const float NdotL = std::abs(PM::pm_Dot3D(dir, sc.N));

			if (NdotL > PM_EPSILON)
			{
				Spectrum app = sc.Material->eval(sc, dir, NdotL)*NdotL;

				Ray ray = in.next(sc.P, dir);
				ShaderClosure sc2;
				RenderEntity* entity = context->shootWithEmission(full_weight, ray, sc2);
				if (entity && sc2.Material)
					full_weight += firstPass(weight*app, ray, sc2, context);
				full_weight *= app;
			}
		}
		else
		{
			pdf = 0;
			float inf_pdf;
			Spectrum inf_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::power(full_weight, pdf, inf_weight, inf_pdf);
		
#if 0 //def PR_DEBUG
			PR_LOGGER.logf(L_Debug, M_Integrator, "  ~~ HitPoint: (%.2f,%.2f)[%.3f,%.3f,%.3f]",
				PM::pm_GetX(in.pixel()), PM::pm_GetY(in.pixel()),
				PM::pm_GetX(sc.P), PM::pm_GetY(sc.P), PM::pm_GetZ(sc.P));
#endif
			// Store it into Hitpoints
			RayHitPoint hitpoint;
			hitpoint.SC = sc;
			hitpoint.PixelX = PM::pm_GetX(in.pixel()); hitpoint.PixelY = PM::pm_GetY(in.pixel());
			hitpoint.Weight = weight;

			hitpoint.CurrentRadius = context->renderer()->settings().ppm().maxGatherRadius() * context->renderer()->settings().ppm().maxGatherRadius();
			hitpoint.CurrentPhotons = 0;

			mThreadData[context->threadNumber()].HitPoints.push_back(hitpoint);
		}

		return full_weight;
	}
	
	Spectrum PPMIntegrator::otherPass(const Ray& in, const ShaderClosure& sc, RenderContext* context)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float pdf;
		PM::vec3 dir = sc.Material->sample(sc, PM::pm_Zero(), pdf);

		Spectrum full_weight;
		if(std::isinf(pdf))// Specular
		{
			const float NdotL = std::abs(PM::pm_Dot3D(dir, sc.N));

			if (NdotL > PM_EPSILON)
			{
				Spectrum app = sc.Material->eval(sc, dir, NdotL)*NdotL;

				Ray ray = in.next(sc.P, dir);
				ShaderClosure sc2;
				RenderEntity* entity = context->shootWithEmission(full_weight, ray, sc2);
				if (entity && sc2.Material)
					full_weight += otherPass(ray, sc2, context);
				full_weight *= app;
			}
		}
		else
		{
			pdf = 0;
			float inf_pdf;
			Spectrum inf_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::power(full_weight, pdf, inf_weight, inf_pdf);
		}

		return full_weight;
	}
}