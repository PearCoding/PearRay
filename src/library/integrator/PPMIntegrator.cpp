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

#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderThread.h"
#include "renderer/OutputMap.h"

#include "spectral/RGBConverter.h"

#include "Logger.h"

//#define PR_ENABLE_DIAGNOSIS
#include "Diagnosis.h"

namespace PR
{
	constexpr uint32 MAX_THETA_SIZE = 128;
	constexpr uint32 MAX_PHI_SIZE = MAX_THETA_SIZE*2;

	PPMIntegrator::PPMIntegrator() :
		Integrator(), mRenderer(nullptr),
		mPhotonMap(nullptr), mThreadData(nullptr),
		mProjMaxTheta(MAX_THETA_SIZE), mProjMaxPhi(MAX_PHI_SIZE),
		mMaxPhotonsStoredPerPass(0), mPhotonsEmitted(0), mPhotonsStored(0)
	{
	}

	PPMIntegrator::~PPMIntegrator()
	{
		if(mPhotonMap)
			delete mPhotonMap;

		if(mThreadData)
		{
			PR_ASSERT(mRenderer, "When ThreadData is non null, renderer has to be non null aswell.");
			delete[] mThreadData;
		}

		for(const auto& l : mLights)
		{
			if(l.Proj)
				delete l.Proj;
		}

		mLights.clear();
	}

	void PPMIntegrator::init(RenderContext* renderer)
	{
		PR_ASSERT(!mPhotonMap, "PhotonMap should be null at first.");
		PR_ASSERT(!mThreadData, "ThreadData should be null at first.");
		PR_ASSERT(mLights.empty(), "Lights should be empty at first.");
		PR_ASSERT(renderer, "Parameter 'renderer' should be valid.");

		PR_ASSERT(renderer->settings().ppm().maxPhotonsPerPass() > 0, "maxPhotonsPerPass should be bigger than 0 and be handled in upper classes.");
		PR_ASSERT(renderer->settings().ppm().maxGatherCount() > 0, "maxGatherCount should be bigger than 0 and be handled in upper classes.");
		PR_ASSERT(renderer->settings().ppm().maxGatherRadius() > PM_EPSILON, "maxGatherRadius should be bigger than 0 and be handled in upper classes.");

		mRenderer = renderer;
		mMaxPhotonsStoredPerPass = renderer->settings().ppm().maxPhotonsPerPass()
				* (renderer->settings().maxDiffuseBounces()+1);
		PR_LOGGER.logf(L_Info, M_Integrator, "Photons to store per pass: %llu", mMaxPhotonsStoredPerPass);

		mPhotonMap = new Photon::PointMap(0.01f);

		mThreadData = new ThreadData[mRenderer->threads()];
		mPhotonsEmitted=0;
		mPhotonsStored=0;

		// Assign photon count to each light
		mProjMaxTheta = PM::pm_Max<uint32>(8, MAX_THETA_SIZE*renderer->settings().ppm().projectionMapQuality());
		mProjMaxPhi = PM::pm_Max<uint32>(8, MAX_PHI_SIZE*renderer->settings().ppm().projectionMapQuality());

		const uint64 Photons = renderer->settings().ppm().maxPhotonsPerPass();
		constexpr uint64 MinPhotons = 10;
		const std::list<RenderEntity*>& lightList = mRenderer->lights();

		const uint64 k = MinPhotons * lightList.size();
		if (k >= Photons) // Not enough photons given.
		{
			PR_LOGGER.logf(L_Warning, M_Integrator, "Not enough photons per pass given. At least %llu is needed.", k);

			for(RenderEntity* light : lightList)
				mLights.push_back(Light({light, MinPhotons, light->surfaceArea(nullptr)}));
		}
		else
		{
			const uint64 d = Photons - k;

			float fullArea = 0;
			for(RenderEntity* light : lightList)
				fullArea += light->surfaceArea(nullptr);

			for(RenderEntity* light : lightList)
			{
				SphereMap* map = nullptr;
				if(renderer->settings().ppm().projectionMapWeight() > PM_EPSILON)
					map = new SphereMap(mProjMaxTheta, mProjMaxPhi);

				const float surface = light->surfaceArea(nullptr);
				Light l({light,
					MinPhotons + (uint64)std::ceil(d * (surface / fullArea)),
					surface,
					map});
				mLights.push_back(l);

				PR_LOGGER.logf(L_Info, M_Integrator, "PPM Light %s %llu photons %f m2",
					light->name().c_str(), l.Photons, l.Surface);
			}
		}

		// Spread photons over threads
		const uint64 PhotonsPerThread = std::ceil(Photons/(float)mRenderer->threads());
		PR_LOGGER.logf(L_Info, M_Integrator, "Each thread shoots %llu photons",
					PhotonsPerThread);
		
		uint64 calcPhotons = 0;
		uint64 calcPhotonsPerLight = 0;
		uint32 currentLight = 0;
		for(uint32 thread = 0; thread < mRenderer->threads(); ++thread)
		{
			uint64 currentPhotons = 0;
			while(calcPhotons < Photons &&
				currentPhotons < PhotonsPerThread &&
				currentLight < mLights.size())
			{
				LightThreadData ltd;
				ltd.Entity = &mLights[currentLight];
				ltd.Photons = std::min(PhotonsPerThread, mLights[currentLight].Photons-calcPhotonsPerLight);

				mThreadData[thread].Lights.push_back(ltd);

				calcPhotonsPerLight += ltd.Photons;
				calcPhotons += ltd.Photons;
				currentPhotons += ltd.Photons;

				if(ltd.Photons < PhotonsPerThread)
				{
					currentLight++;
					calcPhotonsPerLight = 0;
				}
			}

			PR_LOGGER.logf(L_Info, M_Integrator, "PPM Thread %llu lights",
				mThreadData[thread].Lights.size());
			for(const auto& ltd: mThreadData[thread].Lights)
			{
				PR_LOGGER.logf(L_Info, M_Integrator, "  -> Light %s with %llu photons",
					ltd.Entity->Entity->name().c_str(), ltd.Photons);
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
		for(const Light& l : mLights)
		{
			if(!l.Proj)
				continue;

			const Sphere outerSphere = l.Entity->worldBoundingBox().outerSphere();

			for(uint32 thetaI = 0; thetaI < mProjMaxTheta; ++thetaI)
			{
				const float theta = PM_PI_F * thetaI / (float)mProjMaxTheta;
				float thCos, thSin;
				PM::pm_SinCos(theta, thSin, thCos);

				for(uint32 phiI = 0; phiI < mProjMaxPhi; ++phiI)
				{
					const float phi = PM_2_PI_F * phiI / (float)mProjMaxPhi;
					float phCos, phSin;
					PM::pm_SinCos(phi, phSin, phCos);
					PM::vec3 dir = PM::pm_Set(thSin * phCos, thSin * phSin, thCos);

					Ray ray(0,0,
						PM::pm_Add(outerSphere.position(), PM::pm_Scale(dir, outerSphere.radius() + SAFE_DISTANCE)),
						PM::pm_Negate(dir));
					FaceSample lightSample;
					float weight = 0;
					if(l.Entity->checkCollision(ray, lightSample))
					{
						ray = Ray::safe(0,0, lightSample.P, dir, 0, 0, RF_Light);
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

					l.Proj->setProbabilityWithIndex(thetaI, phiI, (weight+0.01f) * DELTA_W + (1-DELTA_W));
				}
			}

			l.Proj->setup();
		}
	}

	void PPMIntegrator::onNextPass(uint32 pass, bool& clean)
	{
		clean = true;// Clear sample, error, etc information prior next pass.
		PR_LOGGER.logf(L_Info, M_Integrator, "Preparing PPM pass %i", pass + 1);

		if (pass % 2)// Photon Pass
		{
			mPhotonMap->reset();
		
			if(pass > 1)
				PR_LOGGER.logf(L_Info, M_Integrator,
					"Already %llu photons emmitted and %llu photons computed",
					mPhotonsEmitted, mPhotonsStored);
		}
	}

	void PPMIntegrator::onEnd()
	{
	}

	bool PPMIntegrator::needNextPass(uint32 pass) const
	{
		return pass < mRenderer->settings().ppm().maxPassCount()*2 + 1;
	}

	void PPMIntegrator::onThreadStart(RenderThreadContext* context)
	{
	}

	void PPMIntegrator::onPrePass(RenderThreadContext* context, uint32 pass)
	{
		if(pass > 0)
		{
			if(pass % 2)
				onPhotonPass(context, pass);
			else
				onAccumPass(context, pass);
		}
	}

	void PPMIntegrator::onPass(RenderTile* tile, RenderThreadContext* context, uint32 pass)
	{
		// TODO: Find a solution for the background.
		
		if(pass == 0)
		{
			for (uint32 y = tile->sy(); y < tile->ey() && !context->thread()->shouldStop(); ++y)
			{
				for (uint32 x = tile->sx(); x < tile->ex() && !context->thread()->shouldStop(); ++x)
				{
					context->render(x, y, tile->samplesRendered(), pass);
				}
			}
		}
	}

	void PPMIntegrator::onPostPass(RenderThreadContext* context, uint32 i)
	{
	}

	void PPMIntegrator::onThreadEnd(RenderThreadContext* context)
	{
	}

	uint64 PPMIntegrator::maxSamples(const RenderContext* renderer) const
	{
		return renderer->width() * renderer->height() *
			renderer->settings().maxPixelSampleCount() * (renderer->settings().ppm().maxPassCount() + 1);
	}

	uint64 PPMIntegrator::maxPasses(const RenderContext* renderer) const
	{
		return renderer->settings().ppm().maxPassCount() + 1;
	}

	void PPMIntegrator::onPhotonPass(RenderThreadContext* context, uint32 pass)
	{
#ifdef PR_DEBUG
		PR_LOGGER.log(L_Debug, M_Integrator, "Shooting Photons");
#endif
		const uint32 H = mRenderer->settings().maxDiffuseBounces()+1;
		const uint32 RD = mRenderer->settings().maxRayDepth();

		Random random;
		for (const LightThreadData& ltd : mThreadData[context->threadNumber()].Lights)
		{
			const Light& light = *ltd.Entity;
			const Sphere outerSphere = light.Entity->worldBoundingBox().outerSphere();

			const size_t sampleSize = ltd.Photons;
			MultiJitteredSampler sampler(random, sampleSize);

			const float inv = 1.0f/sampleSize;
			size_t photonsShoot = 0;
			size_t photonsStored = 0;
			for (photonsShoot = 0;
				 photonsShoot < sampleSize;
				 ++photonsShoot)
			{
				float area_pdf = 0;
				float t_pdf = 0;
				FaceSample lightSample;
				PM::vec3 dir;

				if(light.Proj)
				{
					dir = light.Proj->sample(random.getFloat(), random.getFloat(), random.getFloat(), random.getFloat(), area_pdf);
					t_pdf = 1;

					Ray ray(0,0,
							PM::pm_Add(outerSphere.position(), PM::pm_Scale(dir, outerSphere.radius() + SAFE_DISTANCE)),
						PM::pm_Negate(dir));
					if(!light.Entity->checkCollision(ray, lightSample))
						continue;
				}
				else
				{
					lightSample = light.Entity->getRandomFacePoint(sampler, photonsShoot, area_pdf);

					/*dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
								Projection::cos_hemi(random.getFloat(), random.getFloat(), t_pdf));*/
					dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
								Projection::hemi(random.getFloat(), random.getFloat(), t_pdf));
				}

				if(!lightSample.Material->isLight())
					continue;

				Ray ray = Ray::safe(0, 0, lightSample.P, dir, 0, 0, RF_Light);
				ShaderClosure lsc = lightSample;
				Spectrum radiance = lightSample.Material->emission()->eval(lsc);
				//radiance /= t_pdf;// This is not working properly

				PR_CHECK_VALIDITY(radiance, "After photon emission");

				mPhotonsEmitted++;

				uint32 diffuseBounces = 0;
				for (uint32 j = 0; j < RD; ++j)
				{
					ShaderClosure sc;
					RenderEntity* entity = mRenderer->shoot(ray, sc, nullptr);

					if (entity && sc.Material && sc.Material->canBeShaded())
					{
						//if(j == 0)
						//	radiance /= MSI::toSolidAngle(area_pdf, sc.Depth2, std::abs(sc.NdotV));

						float pdf;
						PM::vec3 nextDir = sc.Material->sample(sc, random.get3D(), pdf);

						if (pdf > PM_EPSILON &&
							!std::isinf(pdf) &&
							!(sc.Flags & SCF_Inside))// Diffuse
						{
							// Always store when diffuse
							Photon::Photon photon;
							PM::pm_Store3D(sc.P, photon.Position);
							mPhotonMap->mapDirection(PM::pm_Negate(ray.direction()),
								photon.Theta, photon.Phi);
#if PR_PHOTON_RGB_MODE >= 1
							RGBConverter::convert(radiance*inv,
								photon.Power[0], photon.Power[1], photon.Power[2]);
#else
							photon.Power = radiance*inv;
#endif
							mPhotonMap->store(sc.P, photon);

							mPhotonsStored++;
							photonsStored++;
							diffuseBounces++;

							if (diffuseBounces > H-1)
								break;// Absorb
						}
						else if(!std::isinf(pdf))// Absorb
						{
							break;
						}

						const float NdotL = std::abs(PM::pm_Dot3D(nextDir, sc.N));
						radiance *= PR_CHECK_VALIDITY(sc.Material->eval(sc, nextDir, NdotL), "After material eval")
							* (NdotL / (std::isinf(pdf) ? 1 : pdf));
						ray = ray.next(sc.P, nextDir);

						PR_CHECK_VALIDITY(std::isinf(pdf) ? 1 : pdf, "After photon apply");
						PR_CHECK_VALIDITY(radiance, "After photon apply");
					}
					else // Nothing found, abort
					{
						break;
					}
				}
			}
		}
	}

	void PPMIntegrator::onAccumPass(RenderThreadContext* context, uint32 pass)
	{
#ifdef PR_DEBUG
		PR_LOGGER.log(L_Debug, M_Integrator, "Accumulating Photons");
#endif
		const auto gatheringMode = context->renderer()->settings().ppm().gatheringMode();
		const float A = 1 - context->renderer()->settings().ppm().contractRatio();

		Spectrum full_weight;
		Photon::PointSphere query;
		query.MaxPhotons = context->renderer()->settings().ppm().maxGatherCount();
		query.SqueezeWeight = context->renderer()->settings().ppm().squeezeWeight() *
			context->renderer()->settings().ppm().squeezeWeight();

		for(RayHitPoint& p : mThreadData[context->threadNumber()].HitPoints)
		{
			query.Center = p.SC.P;
			query.Normal = p.SC.N;
			query.Distance2 = p.CurrentRadius;

			auto accumFunc = [&](const Photon::Photon& photon, const Photon::PointSphere& sp, float d2)
			{
				const PM::vec3 dir = mPhotonMap->evalDirection(photon.Theta, photon.Phi);

				const float NdotL = std::abs(PM::pm_Dot3D(p.SC.N, dir));
				#if PR_PHOTON_RGB_MODE >= 1
					return PR_CHECK_VALIDITY(p.SC.Material->eval(p.SC, dir, NdotL), "After photon material eval")
						* PR_CHECK_VALIDITY(
								RGBConverter::toSpec(photon.Power[0], photon.Power[1], photon.Power[2]),
								"After photon power convert");
				#else
					return PR_CHECK_VALIDITY(p.SC.Material->eval(p.SC, dir, NdotL), "After photon material eval")
						* PR_CHECK_VALIDITY(photon.Power, "After photon power convert");
				#endif//PR_PHOTON_RGB_MODE
			};

			size_t found=0;
			switch (gatheringMode)
			{
			default:
			case PGM_Sphere:
				full_weight = mPhotonMap->estimateSphere(query, accumFunc, found);
				break;
			case PGM_Dome:
				full_weight = mPhotonMap->estimateDome(query, accumFunc, found);
				break;
			}

			if(found > 1)
			{
				// Change radius, photons etc.
				const uint64 newN = p.CurrentPhotons + std::floor(A*found);
				const float fraction = newN / (float)(p.CurrentPhotons + found);
				p.CurrentRadius *= fraction;
				p.CurrentPhotons = newN;

#if PR_PHOTON_RGB_MODE == 2
				float rgb[3];
				RGBConverter::convert(full_weight, rgb[0], rgb[1], rgb[2]);
				p.CurrentFlux[0] = (p.CurrentFlux[0] + rgb[0]) * fraction;
				p.CurrentFlux[1] = (p.CurrentFlux[1] + rgb[1]) * fraction;
				p.CurrentFlux[2] = (p.CurrentFlux[2] + rgb[2]) * fraction;

				PR_CHECK_VALIDITY(p.CurrentFlux[0], "After photon to current flux [0]");
				PR_CHECK_VALIDITY(p.CurrentFlux[1], "After photon to current flux [1]");
				PR_CHECK_VALIDITY(p.CurrentFlux[2], "After photon to current flux [2]");
#else
				p.CurrentFlux = (p.CurrentFlux + full_weight) * fraction;
				PR_CHECK_VALIDITY(p.CurrentFlux, "After photon to current flux");
#endif//PR_PHOTON_RGB_MODE
			}

			const float inv = PM_INV_PI_F / p.CurrentRadius;

			// Render result:
			Spectrum est;
#if PR_PHOTON_RGB_MODE == 2
			est = RGBConverter::toSpec(p.Weight[0] * p.CurrentFlux[0] * inv,
				p.Weight[1] * p.CurrentFlux[1] * inv,
				p.Weight[2] * p.CurrentFlux[2] * inv);
#else
			est = p.Weight * p.CurrentFlux * inv;
#endif//PR_PHOTON_RGB_MODE
			context->renderer()->output()->pushFragment(p.PixelX, p.PixelY, est, p.SC);

			PR_CHECK_VALIDITY(inv, "After photon estimation");
			PR_CHECK_VALIDITY(est, "After photon estimation");
		}
	}

	Spectrum PPMIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
	{
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		PR_CHECK_VALIDITY(applied, "From emission");
		if (!entity || !sc.Material)
			return applied;

		return applied + PR_CHECK_VALIDITY(firstPass(Spectrum(1), in, sc, context), "From firstPass");
	}

	Spectrum PPMIntegrator::firstPass(const Spectrum& weight, const Ray& in, const ShaderClosure& sc, RenderThreadContext* context)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		PR_CHECK_VALIDITY(weight, "Before first pass");
#if PR_PHOTON_RGB_MODE == 2
		float cpWeight[3];
		RGBConverter::convert(weight, cpWeight[0], cpWeight[1], cpWeight[2]);
		PR_CHECK_VALIDITY(cpWeight[0], "Before first pass: cpWeight [0]");
		PR_CHECK_VALIDITY(cpWeight[1], "Before first pass: cpWeight [1]");
		PR_CHECK_VALIDITY(cpWeight[2], "Before first pass: cpWeight [2]");
#endif

		Spectrum full_weight;
		const PM::vec3 rnd = context->random().get3D();
		for(uint32 path = 0; path < sc.Material->samplePathCount(); ++path)
		{
			float pdf;
			Spectrum path_weight;
			PM::vec3 dir = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);

			if(pdf <= PM_EPSILON)
				continue;

			Spectrum other_weight;
			if(std::isinf(pdf))// Specular
			{
				const float NdotL = std::abs(PM::pm_Dot3D(dir, sc.N));

				if (NdotL > PM_EPSILON)
				{
					Spectrum app = sc.Material->eval(sc, dir, NdotL)*NdotL;
					PR_CHECK_VALIDITY(app, "After ray eval");

					Ray ray = in.next(sc.P, dir);
					ShaderClosure sc2;
					RenderEntity* entity = context->shootWithEmission(other_weight, ray, sc2);
					if (entity && sc2.Material)
						other_weight += firstPass(path_weight * (other_weight + weight*app), ray, sc2, context);
					other_weight *= app;
					PR_CHECK_VALIDITY(other_weight, "After ray apply");
				}
			}
			else
			{
				pdf = 0;
				float inf_pdf;
				Spectrum inf_weight = handleInfiniteLights(in, sc, context, inf_pdf);
				MSI::power(other_weight, pdf, inf_weight, inf_pdf);

				PR_CHECK_VALIDITY(inf_pdf, "After inf lights");
				PR_CHECK_VALIDITY(inf_weight, "After inf lights");

#if 0 //def PR_DEBUG
				PR_LOGGER.logf(L_Debug, M_Integrator, "  ~~ HitPoint: (%.2f,%.2f)[%.3f,%.3f,%.3f]",
					PM::pm_GetX(in.pixel()), PM::pm_GetY(in.pixel()),
					PM::pm_GetX(sc.P), PM::pm_GetY(sc.P), PM::pm_GetZ(sc.P));
#endif
				// Store it into Hitpoints
				RayHitPoint hitpoint;
				hitpoint.SC = sc;
				hitpoint.PixelX = in.pixelX();
				hitpoint.PixelY = in.pixelY();

#if PR_PHOTON_RGB_MODE == 2
				hitpoint.Weight[0] = cpWeight[0]; hitpoint.Weight[1] = cpWeight[1]; hitpoint.Weight[2] = cpWeight[2];
				hitpoint.CurrentFlux[0] = 0; hitpoint.CurrentFlux[1] = 0; hitpoint.CurrentFlux[2] = 0;
#else
				hitpoint.Weight = weight;
#endif//PR_PHOTON_RGB_MODE

				hitpoint.CurrentRadius = context->renderer()->settings().ppm().maxGatherRadius() * context->renderer()->settings().ppm().maxGatherRadius();
				hitpoint.CurrentPhotons = 0;

				mThreadData[context->threadNumber()].HitPoints.push_back(hitpoint);
			}

			full_weight += path_weight * other_weight;

			PR_CHECK_VALIDITY(full_weight, "After path accumulation");
		}

		return full_weight;
	}
}
