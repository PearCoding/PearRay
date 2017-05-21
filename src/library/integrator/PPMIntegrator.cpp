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

#include "Diagnosis.h"

namespace PR
{
	constexpr uint32 MAX_THETA_SIZE = 128;
	constexpr uint32 MAX_PHI_SIZE = MAX_THETA_SIZE*2;

	PPMIntegrator::PPMIntegrator(RenderContext* renderer) :
		Integrator(renderer),
		mPhotonMap(nullptr), mThreadData(nullptr),
		mAccumulatedFlux(nullptr), mSearchRadius2(nullptr), mLocalPhotonCount(nullptr),
		mProjMaxTheta(MAX_THETA_SIZE), mProjMaxPhi(MAX_PHI_SIZE),
		mMaxPhotonsStoredPerPass(0)
	{
		PR_ASSERT(renderer, "Parameter 'renderer' should be valid.");
	}

	PPMIntegrator::~PPMIntegrator()
	{
		if(mPhotonMap)
			delete mPhotonMap;

		if(mThreadData)
			delete[] mThreadData;

		for(const auto& l : mLights)
		{
			if(l.Proj)
				delete l.Proj;
		}

		mLights.clear();
	}

	void PPMIntegrator::init()
	{
		PR_ASSERT(!mPhotonMap, "PhotonMap should be null at first.");
		PR_ASSERT(!mThreadData, "ThreadData should be null at first.");
		PR_ASSERT(mLights.empty(), "Lights should be empty at first.");

		PR_ASSERT(renderer()->settings().ppm().maxPhotonsPerPass() > 0, "maxPhotonsPerPass should be bigger than 0 and be handled in upper classes.");
		PR_ASSERT(renderer()->settings().ppm().maxGatherCount() > 0, "maxGatherCount should be bigger than 0 and be handled in upper classes.");
		PR_ASSERT(renderer()->settings().ppm().maxGatherRadius() > PR_EPSILON, "maxGatherRadius should be bigger than 0 and be handled in upper classes.");

		mMaxPhotonsStoredPerPass = renderer()->settings().ppm().maxPhotonsPerPass()
				* (renderer()->settings().maxDiffuseBounces()+1);
		PR_LOGGER.logf(L_Info, M_Integrator, "Photons to store per pass: %llu", mMaxPhotonsStoredPerPass);

		mPhotonMap = new Photon::PhotonMap(renderer()->settings().ppm().maxGatherRadius());

		mAccumulatedFlux = std::make_shared<OutputSpectral>(renderer(), Spectrum(), true);// Will be deleted by outputmap
		mSearchRadius2 = std::make_shared<Output1D>(renderer(),
			renderer()->settings().ppm().maxGatherRadius() * renderer()->settings().ppm().maxGatherRadius(),
			true);
		mLocalPhotonCount = std::make_shared<OutputCounter>(renderer(), 0, true);
	
		renderer()->output()->registerCustomChannel("int.ppm.accumulated_flux", mAccumulatedFlux);
		renderer()->output()->registerCustomChannel("int.ppm.search_radius", mSearchRadius2);
		renderer()->output()->registerCustomChannel("int.ppm.local_photon_count", mLocalPhotonCount);

		mThreadData = new ThreadData[renderer()->threads()];
		for(uint32 thread = 0; thread < renderer()->threads(); ++thread)
		{
			mThreadData[thread].PhotonsEmitted=0;
			mThreadData[thread].PhotonsStored=0;
		}

		// Assign photon count to each light
		mProjMaxTheta = std::max<uint32>(8, MAX_THETA_SIZE*renderer()->settings().ppm().projectionMapQuality());
		mProjMaxPhi = std::max<uint32>(8, MAX_PHI_SIZE*renderer()->settings().ppm().projectionMapQuality());

		const uint64 Photons = renderer()->settings().ppm().maxPhotonsPerPass();
		const uint64 MinPhotons = renderer()->settings().ppm().maxPhotonsPerPass()*0.1f;// Should be a parameter
		const std::list<RenderEntity*>& lightList = renderer()->lights();

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
				if(renderer()->settings().ppm().projectionMapWeight() > PR_EPSILON)
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
		const uint64 PhotonsPerThread = std::ceil(Photons/(float)renderer()->threads());
		PR_LOGGER.logf(L_Info, M_Integrator, "Each thread shoots %llu photons",
					PhotonsPerThread);
		
		std::vector<uint64> photonsPerThread(renderer()->threads(), 0);
		for(Light& light : mLights)
		{
			uint64 photonsSpread = 0;
			for(uint32 thread = 0; thread < renderer()->threads(); ++thread)
			{
				if(photonsSpread >= light.Photons)
					break;
				
				if(photonsPerThread[thread] >= PhotonsPerThread)
					continue;
				
				const uint64 photons = std::min(PhotonsPerThread - photonsPerThread[thread],
								light.Photons - photonsSpread);
				
				photonsSpread += photons;
				photonsPerThread[thread] += photons;
				
				if (photons > 0)
				{
					LightThreadData ltd;
					ltd.Entity = &light;
					ltd.Photons = photons;
					mThreadData[thread].Lights.push_back(ltd);
				}
			}
		}

		for(uint32 thread = 0; thread < renderer()->threads(); ++thread)
		{
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
		if(renderer()->settings().ppm().projectionMapWeight() <= PR_EPSILON)
			return;

		PR_LOGGER.logf(L_Info, M_Integrator, "Calculating Projection Maps (%i,%i)", mProjMaxTheta, mProjMaxPhi);

		const float DELTA_W = renderer()->settings().ppm().projectionMapWeight();
		const float CAUSTIC_PREF = renderer()->settings().ppm().projectionMapPreferCaustic();

		Random random(renderer()->settings().seed() ^ 0x314ff64e);
		for(const Light& l : mLights)
		{
			if(!l.Proj)
				continue;

			const Sphere outerSphere = l.Entity->worldBoundingBox().outerSphere();

			for(uint32 thetaI = 0; thetaI < mProjMaxTheta; ++thetaI)
			{
				const float theta = PR_PI * thetaI / (float)mProjMaxTheta;
				float thSin = std::sin(theta);
				float thCos = std::cos(theta);

				for(uint32 phiI = 0; phiI < mProjMaxPhi; ++phiI)
				{
					const float phi = 2 * PR_PI * phiI / (float)mProjMaxPhi;
					float phSin = std::sin(phi);
					float phCos = std::cos(phi);
					Eigen::Vector3f dir(thSin * phCos, thSin * phSin, thCos);

					Ray ray(Eigen::Vector2i(0,0),
						outerSphere.position() + dir * (outerSphere.radius() + SAFE_DISTANCE),
						-dir);
					FaceSample lightSample;
					float weight = 0;
					if(l.Entity->checkCollision(ray, lightSample))
					{
						ray = Ray::safe(Eigen::Vector2i(0,0), lightSample.P, dir, 0, 0, 0, RF_Light);
						float pdf = std::abs(dir.dot(lightSample.Ng));

						uint32 j;// Calculates specular count
						for (j = 0;
							j < renderer()->settings().maxRayDepth();
							++j)
						{
							ShaderClosure sc;
							RenderEntity* entity = renderer()->shoot(ray, sc, nullptr);

							if (entity && sc.Material && sc.Material->canBeShaded())
							{
								Eigen::Vector3f nextDir;
								float l_pdf;

								Eigen::Vector3f s = random.get3D();
								nextDir = sc.Material->sample(sc, s, l_pdf);

								const float NdotL = std::abs(nextDir.dot(sc.N));
								if(pdf*NdotL <= PR_EPSILON)// Drop this one.
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
						weight = pdf*(1+(j/(float)renderer()->settings().maxRayDepth())*CAUSTIC_PREF);
					}

					l.Proj->setProbabilityWithIndex(thetaI, phiI, (weight+0.01f) * DELTA_W + (1-DELTA_W));
				}
			}

			l.Proj->setup();
		}
	}

	void PPMIntegrator::onNextPass(uint32 pass, bool& clean)
	{
		// Clear sample, error, etc information prior next pass.
		clean = pass % 2;
		PR_LOGGER.logf(L_Info, M_Integrator, "Preparing PPM pass %i (PP %i, AP %i)", pass + 1,
			pass / 2 + 1, pass / 2 + pass % 2);

		if (pass % 2 == 0)// Photon Pass
			mPhotonMap->reset();
	}

	void PPMIntegrator::onEnd()
	{
	}

	bool PPMIntegrator::needNextPass(uint32 pass) const
	{
		return pass < renderer()->settings().ppm().maxPassCount()*2;
	}

	void PPMIntegrator::onThreadStart(RenderThreadContext* context)
	{
	}

	void PPMIntegrator::onPrePass(RenderThreadContext* context, uint32 pass)
	{
		if(pass % 2 == 0)
			photonPass(context, pass);
	}

	void PPMIntegrator::onPass(RenderTile* tile, RenderThreadContext* context, uint32 pass)
	{
		if(pass % 2 == 1)
		{
			for (uint32 y = tile->sy(); y < tile->ey() && !context->thread()->shouldStop(); ++y)
				for (uint32 x = tile->sx(); x < tile->ex() && !context->thread()->shouldStop(); ++x)
					context->render(Eigen::Vector2i(x, y), tile->samplesRendered(), pass);
		}
	}

	void PPMIntegrator::onPostPass(RenderThreadContext* context, uint32 i)
	{
	}

	void PPMIntegrator::onThreadEnd(RenderThreadContext* context)
	{
	}

	RenderStatus PPMIntegrator::status() const
	{
		const uint64 max_pass_samples =
			renderer()->width() * renderer()->height() *
			renderer()->settings().maxCameraSampleCount();
		const uint64 max_samples = max_pass_samples * renderer()->settings().ppm().maxPassCount();
		RenderStatus stat;

		uint64 photonsEmitted = 0;
		uint64 photonsStored = 0;
		for(uint32 thread = 0; thread < renderer()->threads(); ++thread)
		{
			photonsEmitted += mThreadData[thread].PhotonsEmitted;
			photonsStored += mThreadData[thread].PhotonsStored;
		}

		stat.setField("int.max_sample_count", max_samples);
		stat.setField("int.max_pass_count", (uint64)2*renderer()->settings().ppm().maxPassCount());
		stat.setField("int.photons_emitted", photonsEmitted);
		stat.setField("int.photons_stored", photonsStored);

		if(renderer()->currentPass() % 2 == 0)
			stat.setField("int.pass_name", "Photon");
		else
			stat.setField("int.pass_name", "Accumulation");

		const uint64 PhotonsPerPass = renderer()->settings().ppm().maxPhotonsPerPass();
		const float passEff = 1.0f/(2*renderer()->settings().ppm().maxPassCount());
		float percentage = renderer()->currentPass()*passEff;
		if(renderer()->currentPass()%2 == 0)// Photon Pass
			percentage += passEff * photonsEmitted / ((renderer()->currentPass()/2+1)*PhotonsPerPass);
		else
			percentage += passEff * renderer()->statistics().pixelSampleCount() / (float)max_pass_samples;
		
		stat.setPercentage(percentage);

		return stat;
	}

	void PPMIntegrator::photonPass(RenderThreadContext* context, uint32 pass)
	{
#ifdef PR_DEBUG
		PR_LOGGER.log(L_Debug, M_Integrator, "Shooting Photons");
#endif
		const uint32 H = renderer()->settings().maxDiffuseBounces()+1;
		const uint32 RD = renderer()->settings().maxRayDepth();

		ThreadData& data = mThreadData[context->threadNumber()];
		for (const LightThreadData& ltd : data.Lights)
		{
			const Light& light = *ltd.Entity;
			const Sphere outerSphere = light.Entity->worldBoundingBox().outerSphere();

			const size_t sampleSize = ltd.Photons;
			MultiJitteredSampler sampler(context->random(), sampleSize);

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
				Eigen::Vector3f dir;

				if(light.Proj)
				{
					dir = light.Proj->sample(
						context->random().getFloat(), context->random().getFloat(),
						context->random().getFloat(), context->random().getFloat(), area_pdf);
					t_pdf = 1;

					Ray ray(Eigen::Vector2i(0,0),
							outerSphere.position() + dir*(outerSphere.radius() + SAFE_DISTANCE),
						-dir);
					if(!light.Entity->checkCollision(ray, lightSample))
						continue;
				}
				else
				{
					lightSample = light.Entity->getRandomFacePoint(sampler, photonsShoot, area_pdf);

					/*dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
								Projection::cos_hemi(context->random().getFloat(), context->random().getFloat(), t_pdf));*/
					dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
								Projection::hemi(context->random().getFloat(), context->random().getFloat(), t_pdf));
				}

				if(!lightSample.Material->isLight())
					continue;

				Ray ray = Ray::safe(Eigen::Vector2i(0, 0), lightSample.P, dir, 0, 0, 0, RF_Light);// TODO. Use pixel sample
				ShaderClosure lsc = lightSample;
				Spectrum radiance = lightSample.Material->emission()->eval(lsc);
				//radiance /= t_pdf;// This is not working properly

				PR_CHECK_VALIDITY(radiance, "After photon emission");

				data.PhotonsEmitted++;

				uint32 diffuseBounces = 0;
				for (uint32 j = 0; j < RD; ++j)
				{
					ShaderClosure sc;
					RenderEntity* entity = renderer()->shoot(ray, sc, nullptr);

					if (entity && sc.Material && sc.Material->canBeShaded())
					{
						//if(j == 0)
						//	radiance /= MSI::toSolidAngle(area_pdf, sc.Depth2, std::abs(sc.NdotV));

						float pdf;
						Eigen::Vector3f nextDir = sc.Material->sample(sc, context->random().get3D(), pdf);

						if (pdf > PR_EPSILON &&
							!std::isinf(pdf) &&
							!(sc.Flags & SCF_Inside))// Diffuse
						{
							// Always store when diffuse
							Photon::Photon photon;
							photon.Position[0] = sc.P(0);
							photon.Position[1] = sc.P(1);
							photon.Position[2] = sc.P(2);
							mPhotonMap->mapDirection(-ray.direction(),
								photon.Theta, photon.Phi);
#if PR_PHOTON_RGB_MODE >= 1
							RGBConverter::convert(radiance*inv,
								photon.Power[0], photon.Power[1], photon.Power[2]);
#else
							photon.Power = radiance*inv;
#endif
							mPhotonMap->store(sc.P, photon);

							data.PhotonsStored++;
							photonsStored++;
							diffuseBounces++;

							if (diffuseBounces > H-1)
								break;// Absorb
						}
						else if(!std::isinf(pdf))// Absorb
						{
							break;
						}

						const float NdotL = std::abs(nextDir.dot(sc.N));
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

	Spectrum PPMIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
	{
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		PR_CHECK_VALIDITY(applied, "From emission");
		if (!entity || !sc.Material)
			return applied;

		return applied + PR_CHECK_VALIDITY(accumPass(in, sc, context), "From accumPass");
	}

	Spectrum PPMIntegrator::accumPass(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		Spectrum full_weight;
		const Eigen::Vector3f rnd = context->random().get3D();
		for(uint32 path = 0; path < sc.Material->samplePathCount(); ++path)
		{
			float pdf;
			float path_weight;
			Eigen::Vector3f dir = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);

			if(pdf <= PR_EPSILON)
				continue;

			Spectrum other_weight;
			if(std::isinf(pdf))// Specular
			{
				const float NdotL = std::abs(dir.dot(sc.N));

				if (NdotL > PR_EPSILON)
				{
					Spectrum app = sc.Material->eval(sc, dir, NdotL)*NdotL;
					PR_CHECK_VALIDITY(app, "After ray eval");

					Ray ray = in.next(sc.P, dir);
					ShaderClosure sc2;
					RenderEntity* entity = context->shootWithEmission(other_weight, ray, sc2);
					if (entity && sc2.Material)
						other_weight += accumPass(ray, sc2, context);
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

				// Gathering
				const auto gatheringMode = context->renderer()->settings().ppm().gatheringMode();
				const float A = 1 - context->renderer()->settings().ppm().contractRatio();

				Spectrum accum_weight;
				Photon::PhotonSphere query;

				query.MaxPhotons = context->renderer()->settings().ppm().maxGatherCount();
				query.SqueezeWeight = context->renderer()->settings().ppm().squeezeWeight() *
					context->renderer()->settings().ppm().squeezeWeight();
				query.Center = sc.P;
				query.Normal = sc.N;
				query.Distance2 = mSearchRadius2->getFragment(in.pixel());

				auto accumFunc = [&](const Photon::Photon& photon, const Photon::PhotonSphere& sp, float d2)
				{
					const Eigen::Vector3f dir = mPhotonMap->evalDirection(photon.Theta, photon.Phi);

					const float NdotL = std::abs(sc.N.dot(dir));
					#if PR_PHOTON_RGB_MODE >= 1
						return PR_CHECK_VALIDITY(sc.Material->eval(sc, dir, NdotL), "After photon material eval")
							* PR_CHECK_VALIDITY(
									RGBConverter::toSpec(photon.Power[0], photon.Power[1], photon.Power[2]),
									"After photon power convert");
					#else
						return PR_CHECK_VALIDITY(sc.Material->eval(sc, dir, NdotL), "After photon material eval")
							* PR_CHECK_VALIDITY(photon.Power, "After photon power convert");
					#endif//PR_PHOTON_RGB_MODE
				};

				size_t found=0;
				switch (gatheringMode)
				{
				default:
				case PGM_Sphere:
					accum_weight = mPhotonMap->estimateSphere(query, accumFunc, found);
					break;
				case PGM_Dome:
					accum_weight = mPhotonMap->estimateDome(query, accumFunc, found);
					break;
				}

				if(found > 1)
				{
					const auto currentPhotons = mLocalPhotonCount->getFragment(in.pixel());
					// Change radius, photons etc.
					const uint64 newN = currentPhotons + std::floor(A*found);
					const float fraction = newN / (float)(currentPhotons + found);
					
					mSearchRadius2->setFragment(in.pixel(), query.Distance2*fraction);
					mLocalPhotonCount->setFragment(in.pixel(), newN);

					const auto oldFlux = mAccumulatedFlux->getFragment(in.pixel());

					mAccumulatedFlux->setFragment(in.pixel(),
						 PR_CHECK_VALIDITY((oldFlux + accum_weight) * fraction, "After photon to current flux"));
				}

				const float inv = PR_1_PI / mSearchRadius2->getFragment(in.pixel());

				MSI::power(other_weight, pdf,
					mAccumulatedFlux->getFragment(in.pixel())*inv, 1);

				PR_CHECK_VALIDITY(inv, "After photon estimation");
				PR_CHECK_VALIDITY(other_weight, "After photon estimation");
			}// End diffuse

			full_weight += path_weight * other_weight;

			PR_CHECK_VALIDITY(full_weight, "After path accumulation");
		}

		return full_weight;
	}
}
