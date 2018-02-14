#include "PPMIntegrator.h"

#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

#include "math/MSI.h"
#include "math/Projection.h"

#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"

#include "renderer/OutputMap.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderThread.h"
#include "renderer/RenderTile.h"

#include "spectral/RGBConverter.h"

#include "Logger.h"

namespace PR {
constexpr uint32 MAX_THETA_SIZE = 128;
constexpr uint32 MAX_PHI_SIZE   = MAX_THETA_SIZE * 2;

struct PPM_Light {
	RenderEntity* Entity = nullptr;
	uint64 Photons		 = 0;
	float Surface		 = 0.0f;

	PPM_Light(RenderEntity* entity, uint64 photons, float surface)
		: Entity(entity)
		, Photons(photons)
		, Surface(surface)
	{
	}
};

struct PPM_LightTileData {
	PPM_Light* Entity = nullptr;
	uint64 Photons	= 0;
};

struct PPM_TileData {
	std::vector<PPM_LightTileData> Lights;
	uint64 PhotonsEmitted = 0;
	uint64 PhotonsStored  = 0;
};

struct PPM_ThreadData {
	std::vector<Spectrum> FullWeight;
	std::vector<Spectrum> Weight;
	std::vector<Spectrum> Evaluation;
	std::vector<Spectrum> Accum;

	PPM_ThreadData(RenderContext* context)
		: FullWeight(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Weight(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Evaluation(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Accum(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
	{
	}
};

PPMIntegrator::PPMIntegrator(RenderContext* renderer)
	: Integrator(renderer)
	, mPhotonMap(nullptr)
	, mAccumulatedFlux(nullptr)
	, mSearchRadius2(nullptr)
	, mLocalPhotonCount(nullptr)
	, mMaxPhotonsStoredPerPass(0)
{
	PR_ASSERT(renderer, "Parameter 'renderer' should be valid.");
}

PPMIntegrator::~PPMIntegrator()
{
	if (mPhotonMap)
		delete mPhotonMap;

	mLights.clear();
}

void PPMIntegrator::init()
{
	Integrator::init();

	PR_ASSERT(!mPhotonMap, "PhotonMap should be null at first.");
	PR_ASSERT(mLights.empty(), "Lights should be empty at first.");

	PR_ASSERT(renderer()->settings().ppm().maxPhotonsPerPass() > 0, "maxPhotonsPerPass should be bigger than 0 and be handled in upper classes.");
	PR_ASSERT(renderer()->settings().ppm().maxGatherCount() > 0, "maxGatherCount should be bigger than 0 and be handled in upper classes.");
	PR_ASSERT(renderer()->settings().ppm().maxGatherRadius() > PR_EPSILON, "maxGatherRadius should be bigger than 0 and be handled in upper classes.");

	mMaxPhotonsStoredPerPass = renderer()->settings().ppm().maxPhotonsPerPass() * (renderer()->settings().maxDiffuseBounces() + 1);
	PR_LOGGER.logf(L_Info, M_Integrator, "Photons to store per pass: %llu", mMaxPhotonsStoredPerPass);

	mPhotonMap = new Photon::PhotonMap(renderer()->settings().ppm().maxGatherRadius());

	mAccumulatedFlux = std::make_shared<FrameBufferFloat>(renderer()->spectrumDescriptor()->samples(), 0.0f, true); // Will be deleted by outputmap
	mSearchRadius2   = std::make_shared<FrameBufferFloat>(1, renderer()->settings().ppm().maxGatherRadius() * renderer()->settings().ppm().maxGatherRadius(),
														true);
	mLocalPhotonCount = std::make_shared<FrameBufferUInt64>(1, 0, true);

	renderer()->output()->registerCustomChannel_Spectral("int.ppm.accumulated_flux", mAccumulatedFlux);
	renderer()->output()->registerCustomChannel_1D("int.ppm.search_radius", mSearchRadius2);
	renderer()->output()->registerCustomChannel_Counter("int.ppm.local_photon_count", mLocalPhotonCount);

	for (uint32 t = 0; t < renderer()->tileCount(); ++t)
		mTileData.emplace_back();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());

	// Assign photon count to each light

	const uint64 Photons					  = renderer()->settings().ppm().maxPhotonsPerPass();
	const uint64 MinPhotons					  = renderer()->settings().ppm().maxPhotonsPerPass() * 0.1f; // Should be a parameter
	const std::list<RenderEntity*>& lightList = renderer()->lights();

	const uint64 k = MinPhotons * lightList.size();
	if (k >= Photons) // Not enough photons given.
	{
		PR_LOGGER.logf(L_Warning, M_Integrator, "Not enough photons per pass given. At least %llu is needed.", k);

		for (RenderEntity* light : lightList)
			mLights.push_back(PPM_Light(light, MinPhotons, light->surfaceArea(nullptr)));
	} else {
		const uint64 d = Photons - k;

		float fullArea = 0;
		for (RenderEntity* light : lightList)
			fullArea += light->surfaceArea(nullptr);

		for (RenderEntity* light : lightList) {
			const float surface = light->surfaceArea(nullptr);
			mLights.emplace_back(light,
								 MinPhotons + (uint64)std::ceil(d * (surface / fullArea)),
								 surface);

			PR_LOGGER.logf(L_Info, M_Integrator, "PPM Light %s %llu photons %f m2",
						   light->name().c_str(), mLights.back().Photons, mLights.back().Surface);
		}
	}

	// Spread photons over tile
	const uint64 PhotonsPerTile = std::ceil(Photons / (float)renderer()->tileCount());
	PR_LOGGER.logf(L_Info, M_Integrator, "Each tile shoots %llu photons",
				   PhotonsPerTile);

	std::vector<uint64> photonsPerTile(renderer()->tileCount(), 0);
	for (PPM_Light& light : mLights) {
		uint64 photonsSpread = 0;
		for (uint32 t = 0; t < renderer()->tileCount(); ++t) {
			if (photonsSpread >= light.Photons)
				break;

			if (photonsPerTile[t] >= PhotonsPerTile)
				continue;

			const uint64 photons = std::min(PhotonsPerTile - photonsPerTile[t],
											light.Photons - photonsSpread);

			photonsSpread += photons;
			photonsPerTile[t] += photons;

			if (photons > 0) {
				PPM_LightTileData ltd;
				ltd.Entity  = &light;
				ltd.Photons = photons;
				mTileData[t].Lights.push_back(ltd);
			}
		}
	}

	for (uint32 t = 0; t < renderer()->tileCount(); ++t) {
		PR_LOGGER.logf(L_Info, M_Integrator, "PPM Tile %llu lights",
					   mTileData[t].Lights.size());
		for (const auto& ltd : mTileData[t].Lights) {
			PR_LOGGER.logf(L_Info, M_Integrator, "  -> Light %s with %llu photons",
						   ltd.Entity->Entity->name().c_str(), ltd.Photons);
		}
	}
}

void PPMIntegrator::onNextPass(uint32 pass, bool& clean)
{
	// Clear sample, error, etc information prior next pass.
	clean = pass % 2;
	PR_LOGGER.logf(L_Info, M_Integrator, "Preparing PPM pass %i (PP %i, AP %i)", pass + 1,
				   pass / 2 + 1, pass / 2 + pass % 2);

	if (pass % 2 == 0) // Photon Pass
		mPhotonMap->reset();
}

void PPMIntegrator::onStart()
{
}

void PPMIntegrator::onEnd()
{
}

bool PPMIntegrator::needNextPass(uint32 pass) const
{
	return pass < renderer()->settings().ppm().maxPassCount() * 2;
}

void PPMIntegrator::onPass(const RenderSession& session, uint32 pass)
{
	if (pass % 2 == 1) {
		const std::unique_ptr<OutputMap>& output = renderer()->output();
		Spectrum spec(renderer()->spectrumDescriptor());
		ShaderClosure sc;

		for (uint32 y = session.tile()->sy(); y < session.tile()->ey(); ++y) {
			for (uint32 x = session.tile()->sx(); x < session.tile()->ex(); ++x) {
				Eigen::Vector2i p(x, y);
				Ray ray = session.tile()->constructCameraRay(p, session.tile()->samplesRendered());
				accumPass(spec, sc, ray, 0, session);
				output->pushFragment(p, spec, sc);
			}
		}
	} else {
		photonPass(session, pass);
	}
}

RenderStatus PPMIntegrator::status() const
{
	const uint64 max_pass_samples = renderer()->width() * renderer()->height() * renderer()->settings().maxCameraSampleCount();
	const uint64 max_samples	  = max_pass_samples * renderer()->settings().ppm().maxPassCount();
	RenderStatus stat;

	uint64 photonsEmitted = 0;
	uint64 photonsStored  = 0;
	for (uint32 thread = 0; thread < renderer()->threads(); ++thread) {
		photonsEmitted += mTileData[thread].PhotonsEmitted;
		photonsStored += mTileData[thread].PhotonsStored;
	}

	stat.setField("int.max_sample_count", max_samples);
	stat.setField("int.max_pass_count", (uint64)2 * renderer()->settings().ppm().maxPassCount());
	stat.setField("int.photons_emitted", photonsEmitted);
	stat.setField("int.photons_stored", photonsStored);

	if (renderer()->currentPass() % 2 == 0)
		stat.setField("int.pass_name", "Photon");
	else
		stat.setField("int.pass_name", "Accumulation");

	const uint64 PhotonsPerPass = renderer()->settings().ppm().maxPhotonsPerPass();
	const float passEff			= 1.0f / (2 * renderer()->settings().ppm().maxPassCount());
	float percentage			= renderer()->currentPass() * passEff;
	if (renderer()->currentPass() % 2 == 0) // Photon Pass
		percentage += passEff * photonsEmitted / ((renderer()->currentPass() / 2 + 1) * PhotonsPerPass);
	else
		percentage += passEff * renderer()->statistics().pixelSampleCount() / (float)max_pass_samples;

	stat.setPercentage(percentage);

	return stat;
}

void PPMIntegrator::photonPass(const RenderSession& session, uint32 pass)
{
#ifdef PR_DEBUG
	PR_LOGGER.log(L_Debug, M_Integrator, "Shooting Photons");
#endif
	const uint32 H  = renderer()->settings().maxDiffuseBounces() + 1;
	const uint32 RD = renderer()->settings().maxRayDepth();

	Spectrum spec(renderer()->spectrumDescriptor());
	Spectrum radiance = spec.clone();
	Spectrum evaluation = spec.clone();

	PPM_TileData& data = mTileData[session.tile()->index()];
	for (const PPM_LightTileData& ltd : data.Lights) {
		const PPM_Light& light		 = *ltd.Entity;

		const size_t sampleSize = ltd.Photons;

		const float inv		 = 1.0f / sampleSize;
		size_t photonsShoot  = 0;
		size_t photonsStored = 0;
		for (photonsShoot = 0;
			 photonsShoot < sampleSize;
			 ++photonsShoot) {
			float t_pdf	= 0;
			FacePoint lightSample;
			Eigen::Vector3f dir;
			const Eigen::Vector3f rnd  = session.tile()->random().get3D();
			const Eigen::Vector3f rnd2 = session.tile()->random().get3D();

			RenderEntity::FacePointSample fps = light.Entity->sampleFacePoint(rnd);

			lightSample = fps.Point;
			dir = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
											Projection::hemi(rnd(0), rnd(1), t_pdf));

			if (!lightSample.Material->isLight())
				continue;

			Ray ray = Ray(Eigen::Vector2i(0, 0), lightSample.P, dir,
								0, rnd2(0), rnd2(1) * spec.samples(), RF_Light); // TODO: Use pixel sample
			ShaderClosure lsc = lightSample;
			lightSample.Material->evalEmission(radiance, lsc, session);
			radiance /= t_pdf;

			data.PhotonsEmitted++;

			uint32 diffuseBounces = 0;
			for (uint32 j = 0; j < RD; ++j) {
				ShaderClosure sc;
				RenderEntity* entity = renderer()->shoot(ray, sc, session);

				if (entity && sc.Material && sc.Material->canBeShaded()) {
					MaterialSample ms = sc.Material->sample(sc, session.tile()->random().get3D(), session);

					if (ms.PDF_S > PR_EPSILON && !ms.isSpecular()) // Diffuse
					{
						// Always store when diffuse
						Photon::Photon photon;
						photon.Position[0] = sc.P(0);
						photon.Position[1] = sc.P(1);
						photon.Position[2] = sc.P(2);
						mPhotonMap->mapDirection(-ray.direction(),
												 photon.Theta, photon.Phi);

						RGBConverter::convert(radiance * inv,
											  photon.Power[0], photon.Power[1], photon.Power[2]);

						mPhotonMap->store(sc.P, photon);

						data.PhotonsStored++;
						photonsStored++;
						diffuseBounces++;

						if (diffuseBounces > H - 1)
							break;				   // Absorb
					} else if (!ms.isSpecular()) { // Absorb
						break;
					}

					const float NdotL = std::abs(ms.L.dot(sc.N));
					sc.Material->eval(evaluation, sc, ms.L, NdotL, session);
					radiance *= evaluation;
					radiance *= (NdotL / ms.PDF_S);
					ray = ray.next(sc.P, ms.L);
				} else { // Nothing found, abort
					break;
				}
			}
		}
	}
}

void PPMIntegrator::accumPass(Spectrum& spec, ShaderClosure& sc, const Ray& in, uint32 diffbounces, const RenderSession& session)
{
	PPM_ThreadData& threadData = mThreadData[session.thread()];
	RenderEntity* entity = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffbounces > renderer()->settings().maxDiffuseBounces())
		return;

	const uint32 depth = in.depth();
	float full_pdf			  = 0;
	const Eigen::Vector3f rnd = session.tile()->random().get3D();
	for (uint32 path = 0; path < sc.Material->samplePathCount(); ++path) {
		MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);

		if (ms.PDF_S <= PR_EPSILON)
			continue;

		if (ms.isSpecular()) { // Specular
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (NdotL > PR_EPSILON) {
				ShaderClosure sc2;

				Ray ray		 = in.next(sc.P, ms.L);
				accumPass(threadData.Weight[depth], sc2, ray, diffbounces, session);
				sc.Material->eval(threadData.Evaluation[depth], sc, ms.L, NdotL, session);
				threadData.Weight[depth] *= threadData.Evaluation[depth] * NdotL;

				MSI::balance(threadData.FullWeight[depth], full_pdf, threadData.Weight[depth], ms.PDF_S);
			}
		} else { // Diffuse
			ms.PDF_S = 0;
			float inf_pdf;
			handleInfiniteLights(threadData.Evaluation[depth], in, sc, session, inf_pdf);
			MSI::balance(threadData.FullWeight[depth], full_pdf, threadData.Evaluation[depth], inf_pdf);

			// Gathering
			const auto gatheringMode = renderer()->settings().ppm().gatheringMode();
			const float A			 = 1 - renderer()->settings().ppm().contractRatio();

			Photon::PhotonSphere query;

			query.MaxPhotons	= renderer()->settings().ppm().maxGatherCount();
			query.SqueezeWeight = renderer()->settings().ppm().squeezeWeight() * renderer()->settings().ppm().squeezeWeight();
			query.Center		= sc.P;
			query.Normal		= sc.N;
			query.Distance2		= mSearchRadius2->getFragment(in.pixel());

			auto accumFunc = [&](Spectrum& accum, const Photon::Photon& photon, const Photon::PhotonSphere& sp, float d2) {
				const Eigen::Vector3f dir = mPhotonMap->evalDirection(photon.Theta, photon.Phi);
				const float NdotL = std::abs(sc.N.dot(dir));

				sc.Material->eval(threadData.Evaluation[depth], sc, dir, NdotL, session);
				RGBConverter::toSpec(threadData.Weight[depth], photon.Power[0], photon.Power[1], photon.Power[2]);

				accum += threadData.Evaluation[depth] * threadData.Weight[depth];
			};

			size_t found = 0;
			switch (gatheringMode) {
			default:
			case PGM_Sphere:
				mPhotonMap->estimateSphere(threadData.Accum[depth], query, accumFunc, found);
				break;
			case PGM_Dome:
				mPhotonMap->estimateDome(threadData.Accum[depth], query, accumFunc, found);
				break;
			}

			if (found > 1) {
				const auto currentPhotons = mLocalPhotonCount->getFragment(in.pixel());
				// Change radius, photons etc.
				const uint64 newN	= currentPhotons + std::floor(A * found);
				const float fraction = newN / (float)(currentPhotons + found);

				mSearchRadius2->setFragment(in.pixel(), 0, query.Distance2 * fraction);
				mLocalPhotonCount->setFragment(in.pixel(), 0, newN);

				mAccumulatedFlux->getFragment(in.pixel(), threadData.Evaluation[depth]);

				threadData.Accum[depth] += threadData.Evaluation[depth];
				threadData.Accum[depth] *= fraction;
				mAccumulatedFlux->setFragment(in.pixel(), threadData.Accum[depth]);
			}

			const float inv = PR_1_PI / mSearchRadius2->getFragment(in.pixel());

			mAccumulatedFlux->getFragment(in.pixel(), threadData.Evaluation[depth]);
			MSI::balance(threadData.FullWeight[depth], full_pdf,
						 threadData.Evaluation[depth] * inv, ms.PDF_S);
		} // End diffuse
	}

	spec += threadData.FullWeight[depth];
}
}
