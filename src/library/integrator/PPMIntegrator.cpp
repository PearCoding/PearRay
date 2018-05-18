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

#include "spectral/XYZConverter.h"

#include "Logger.h"

namespace PR {
// Registy Entries:
static const char* RE_PASS_COUNT	   = "ppm/pass/count";
static const char* RE_PHOTONS_PER_PASS = "ppm/pass/photons";
static const char* RE_GATHER_COUNT	 = "ppm/photons/gather/max_count";
static const char* RE_GATHER_RADIUS	= "ppm/photons/gather/max_radius";
static const char* RE_GATHER_MODE	  = "ppm/photons/gather/mode";
static const char* RE_DIFFUSE_DEPTH	= "ppm/diffuse/max_depth";
static const char* RE_SQUEEZE_WEIGHT   = "ppm/photons/gather/squeeze_weight";
static const char* RE_CONTRACT_RATIO   = "ppm/photons/gather/contract_ratio";

//...
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

	explicit PPM_ThreadData(RenderContext* context)
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

	mPassCount = std::max<uint32>(1,
								  renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																			 RE_PASS_COUNT,
																			 16));
	mPhotonsPerPass = std::max<uint64>(64,
									   renderer()->registry()->getByGroup<uint64>(RG_INTEGRATOR,
																				  RE_PHOTONS_PER_PASS,
																				  100000));

	mMaxGatherCount = std::max<uint64>(64,
									   renderer()->registry()->getByGroup<uint64>(RG_INTEGRATOR,
																				  RE_GATHER_COUNT,
																				  100000));

	mMaxGatherRadius = std::max<float>(PR_EPSILON,
									   renderer()->registry()->getByGroup<float>(RG_INTEGRATOR,
																				 RE_GATHER_RADIUS,
																				 10.0f));

	mGatherMode = renderer()->registry()->getByGroup<PPMGatheringMode>(RG_INTEGRATOR,
																	   RE_GATHER_MODE,
																	   PGM_SPHERE);

	mMaxDiffBounces = renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																 RE_DIFFUSE_DEPTH,
																 1);

	mSqueezeWeight2 = std::max<float>(0, std::min<float>(1,
														 renderer()->registry()->getByGroup<float>(RG_INTEGRATOR,
																								   RE_SQUEEZE_WEIGHT,
																								   1.0f)));
	mSqueezeWeight2 *= mSqueezeWeight2;

	mContractRatio = std::max<float>(0, std::min<float>(1,
														renderer()->registry()->getByGroup<float>(RG_INTEGRATOR,
																								  RE_CONTRACT_RATIO,
																								  0.25f)));

	mMaxPhotonsStoredPerPass = mPhotonsPerPass * mMaxDiffBounces;

	PR_LOG(L_DEBUG) << "Photons to store per pass: " << mMaxPhotonsStoredPerPass << std::endl;

	mPhotonMap = new Photon::PhotonMap(mMaxGatherRadius);

	mAccumulatedFlux  = std::make_shared<FrameBufferFloat>(renderer()->spectrumDescriptor()->samples(), 0.0f, true);
	mSearchRadius2	= std::make_shared<FrameBufferFloat>(1, mMaxGatherRadius * mMaxGatherRadius, true);
	mLocalPhotonCount = std::make_shared<FrameBufferUInt64>(1, 0, true);

	renderer()->output()->registerCustomChannel_Spectral("int.ppm.accumulated_flux", mAccumulatedFlux);
	renderer()->output()->registerCustomChannel_1D("int.ppm.search_radius", mSearchRadius2);
	renderer()->output()->registerCustomChannel_Counter("int.ppm.local_photon_count", mLocalPhotonCount);

	for (uint32 t = 0; t < renderer()->tileCount(); ++t)
		mTileData.emplace_back();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());

	// Assign photon count to each light

	const uint64 Photons	= mPhotonsPerPass;
	const uint64 MinPhotons = mPhotonsPerPass * 0.1f; // Should be a parameter
	const auto& lightList   = renderer()->lights();

	const uint64 k = MinPhotons * lightList.size();
	if (k >= Photons) { // Not enough photons given.
		PR_LOG(L_WARNING) << "Not enough photons per pass given. At least " << k << " is needed." << std::endl;

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
								 MinPhotons + std::ceil(d * (surface / fullArea)),
								 surface);

			PR_LOG(L_DEBUG) << "PPM Light " << light->name() << " " << mLights.back().Photons << " photons " << mLights.back().Surface << "m2" << std::endl;
		}
	}

	// Spread photons over tile
	const uint64 PhotonsPerTile = std::ceil(Photons / static_cast<float>(renderer()->tileCount()));
	PR_LOG(L_DEBUG) << "Each tile shoots " << PhotonsPerTile << " photons" << std::endl;

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
		PR_LOG(L_DEBUG) << "PPM Tile " << mTileData[t].Lights.size() << " lights" << std::endl;
		for (auto ltd : mTileData[t].Lights) {
			PR_LOG(L_DEBUG) << "  -> Light " << ltd.Entity->Entity->name() << " with " << ltd.Photons << " photons" << std::endl;
		}
	}
}

void PPMIntegrator::onNextPass(uint32 pass, bool& clean)
{
	// Clear sample, error, etc information prior next pass.
	clean = pass % 2;
	PR_LOG(L_DEBUG) << "Preparing PPM pass " << (pass + 1) << " (PP " << (pass / 2 + 1) << ", AP " << (pass / 2 + pass % 2) << ")" << std::endl;

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
	return pass < mPassCount * 2;
}

void PPMIntegrator::onPass(const RenderSession& session, uint32 pass)
{
	if (pass % 2 == 1) {
		OutputMap* output = renderer()->output();
		Spectrum spec	 = Spectrum::black(renderer()->spectrumDescriptor());
		ShaderClosure sc;

		for (uint32 y = session.tile()->sy(); y < session.tile()->ey(); ++y) {
			for (uint32 x = session.tile()->sx(); x < session.tile()->ex(); ++x) {
				Eigen::Vector2i p(x, y);
				Ray ray = session.tile()->constructCameraRay(p, session.tile()->samplesRendered());
				spec.clear();
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
	const uint64 max_pass_samples = renderer()->width() * renderer()->height() * renderer()->samplesPerPixel();
	const uint64 max_samples	  = max_pass_samples * mPassCount;
	RenderStatus stat;

	uint64 photonsEmitted = 0;
	uint64 photonsStored  = 0;
	for (uint32 thread = 0; thread < renderer()->threads(); ++thread) {
		photonsEmitted += mTileData[thread].PhotonsEmitted;
		photonsStored += mTileData[thread].PhotonsStored;
	}

	stat.setField("int.max_sample_count", max_samples);
	stat.setField("int.max_pass_count", static_cast<uint64>(2 * mPassCount));
	stat.setField("int.photons_emitted", photonsEmitted);
	stat.setField("int.photons_stored", photonsStored);

	if (renderer()->currentPass() % 2 == 0)
		stat.setField("int.pass_name", "Photon");
	else
		stat.setField("int.pass_name", "Accumulation");

	const float passEff = 1.0f / (2 * mPassCount);
	float percentage	= renderer()->currentPass() * passEff;
	if (renderer()->currentPass() % 2 == 0) // Photon Pass
		percentage += passEff * photonsEmitted / ((renderer()->currentPass() / 2 + 1) * mPhotonsPerPass);
	else
		percentage += passEff * renderer()->statistics().pixelSampleCount() / static_cast<float>(max_pass_samples);

	stat.setPercentage(percentage);

	return stat;
}

void PPMIntegrator::photonPass(const RenderSession& session, uint32 pass)
{
	const uint32 H  = mMaxDiffBounces + 1;
	const uint32 RD = renderer()->settings().maxRayDepth();

	Spectrum radiance(renderer()->spectrumDescriptor(), 0.0f);
	Spectrum evaluation = radiance.clone();

	PPM_TileData& data = mTileData[session.tile()->index()];
	for (const PPM_LightTileData& ltd : data.Lights) {
		const PPM_Light& light = *ltd.Entity;

		const size_t sampleSize = ltd.Photons;

		const float inv		 = 1.0f / sampleSize;
		size_t photonsShoot  = 0;
		size_t photonsStored = 0;
		for (photonsShoot = 0;
			 photonsShoot < sampleSize;
			 ++photonsShoot) {
			float t_pdf				   = 0;
			const Eigen::Vector3f rnd  = session.tile()->random().get3D();
			const Eigen::Vector3f rnd2 = session.tile()->random().get3D();

			RenderEntity::FacePointSample fps = light.Entity->sampleFacePoint(rnd);

			FacePoint lightSample = fps.Point;
			Eigen::Vector3f dir   = Projection::tangent_align(lightSample.Ng, lightSample.Nx, lightSample.Ny,
															Projection::hemi(rnd(0), rnd(1), t_pdf));

			PR_ASSERT(lightSample.Material->isLight(), "Light Sample should always be associated with a light material!");

			Ray ray = Ray(Eigen::Vector2i(0, 0), lightSample.P, dir,
						  0, rnd2(0), rnd2(1) * radiance.samples(), RF_Light); // TODO: Use pixel sample
			lightSample.Material->evalEmission(radiance, ShaderClosure(lightSample), session);
			radiance /= t_pdf;

			data.PhotonsEmitted++;

			uint32 diffuseBounces = 0;
			for (uint32 j = 0; j < RD; ++j) {
				ShaderClosure sc;
				RenderEntity* entity = renderer()->shoot(ray, sc, session);

				if (entity && sc.Material && sc.Material->canBeShaded()) {
					MaterialSample ms = sc.Material->sample(sc, session.tile()->random().get3D(), session);

					if (ms.PDF_S > PR_EPSILON && !ms.isSpecular()) { // Diffuse
						// Always store when diffuse
						Photon::Photon photon;
						for (int k = 0; k < 3; ++k) {
							photon.Position[k]  = sc.P(k);
							photon.Direction[k] = -ray.direction().coeff(k);
						}

						evaluation = radiance * inv;
						XYZConverter::convertXYZ(evaluation,
												 photon.Power[0], photon.Power[1], photon.Power[2]);

						mPhotonMap->store(photon);

						data.PhotonsStored++;
						photonsStored++;
						diffuseBounces++;

						if (diffuseBounces > H - 1)
							break;				   // Absorb
					} else if (!ms.isSpecular()) { // Continue
						ms.PDF_S = 1;
					}

					const float NdotL = std::abs(ms.L.dot(sc.N));
					sc.Material->eval(evaluation, sc, ms.L, NdotL, session);
					radiance *= evaluation * (NdotL / ms.PDF_S);
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
	const float A = 1 - mContractRatio;

	PPM_ThreadData& threadData = mThreadData[session.thread()];
	RenderEntity* entity	   = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffbounces > mMaxDiffBounces)
		return;

	const uint32 depth		  = in.depth();
	float full_pdf			  = 0;
	const Eigen::Vector3f rnd = session.tile()->random().get3D();
	bool specular			  = false;
	for (uint32 path = 0; path < sc.Material->samplePathCount(); ++path) {
		MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);

		if (ms.PDF_S <= PR_EPSILON)
			continue;

		if (ms.isSpecular()) { // Specular
			specular		  = true;
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (NdotL > PR_EPSILON) {
				ShaderClosure sc2;

				Ray ray = in.next(sc.P, ms.L);
				accumPass(threadData.Weight[depth], sc2, ray, diffbounces, session);
				sc.Material->eval(threadData.Evaluation[depth], sc, ms.L, NdotL, session);
				MSI::balance(threadData.FullWeight[depth], full_pdf, threadData.Weight[depth] * threadData.Evaluation[depth] * NdotL, ms.PDF_S);
			}
		} else { // Diffuse
			// Gathering
			Photon::PhotonSphere query;

			query.MaxPhotons	= mMaxGatherCount;
			query.SqueezeWeight = mSqueezeWeight2;
			query.Center		= sc.P;
			query.Normal		= sc.N;
			query.Distance2		= mSearchRadius2->getFragment(in.pixel());

			auto accumFunc = [&](Spectrum& accum, const Photon::Photon& photon, const Photon::PhotonSphere& sp, float d2) {
				const Eigen::Vector3f dir = Eigen::Vector3f(photon.Direction[0], photon.Direction[1], photon.Direction[2]);
				const float NdotL		  = std::abs(sc.N.dot(dir));

				sc.Material->eval(threadData.Evaluation[depth], sc, dir, NdotL, session);
				XYZConverter::toSpec(threadData.Weight[depth], photon.Power[0], photon.Power[1], photon.Power[2]);

				accum += threadData.Evaluation[depth] * threadData.Weight[depth];
			};

			threadData.Accum[depth].clear();
			size_t found = 0;
			switch (mGatherMode) {
			default:
			case PGM_SPHERE:
				mPhotonMap->estimateSphere(threadData.Accum[depth], query, accumFunc, found);
				break;
			case PGM_DOME:
				mPhotonMap->estimateDome(threadData.Accum[depth], query, accumFunc, found);
				break;
			}

			if (found > 1) {
				const auto currentPhotons = mLocalPhotonCount->getFragment(in.pixel());
				// Change radius, photons etc.
				const uint64 newN	= currentPhotons + std::floor(A * found);
				const float fraction = newN / static_cast<float>(currentPhotons + found);

				mSearchRadius2->setFragment(in.pixel(), 0, query.Distance2 * fraction);
				mLocalPhotonCount->setFragment(in.pixel(), 0, newN);
				mAccumulatedFlux->addFragmentSpectrum(in.pixel(), threadData.Accum[depth] * fraction);
			}

			// TODO: Add PPM and other parts together!
			const float inv = PR_1_PI / mSearchRadius2->getFragment(in.pixel());

			mAccumulatedFlux->getFragmentSpectrum(in.pixel(), threadData.Evaluation[depth]);
			MSI::balance(threadData.FullWeight[depth], full_pdf,
						 threadData.Evaluation[depth] * inv, ms.PDF_S);
		} // End diffuse
	}

	/*if (!specular) { // Apply infinite lights if no specular scattering happened
		float inf_pdf;
		handleInfiniteLights(threadData.Evaluation[depth], in, sc, session, inf_pdf);
		MSI::balance(threadData.FullWeight[depth], full_pdf, threadData.Evaluation[depth], inf_pdf);
	}*/

	spec += threadData.FullWeight[depth];
}
} // namespace PR
