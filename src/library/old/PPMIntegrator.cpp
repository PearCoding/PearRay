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
	RenderEntity* VirtualEntity = nullptr;
	uint64 Photons		 = 0;
	float Surface		 = 0.0f;

	PPM_Light(RenderEntity* entity, uint64 photons, float surface)
		: VirtualEntity(entity)
		, Photons(photons)
		, Surface(surface)
	{
	}
};

struct PPM_LightTileData {
	PPM_Light* VirtualEntity = nullptr;
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
				ltd.VirtualEntity  = &light;
				ltd.Photons = photons;
				mTileData[t].Lights.push_back(ltd);
			}
		}
	}

	for (uint32 t = 0; t < renderer()->tileCount(); ++t) {
		PR_LOG(L_DEBUG) << "PPM Tile " << mTileData[t].Lights.size() << " lights" << std::endl;
		for (auto ltd : mTileData[t].Lights) {
			PR_LOG(L_DEBUG) << "  -> Light " << ltd.VirtualEntity->VirtualEntity->name() << " with " << ltd.Photons << " photons" << std::endl;
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
	// Do nothing -- Will be removed
}

void PPMIntegrator::accumPass(Spectrum& spec, ShaderClosure& sc, const Ray& in, uint32 diffbounces, const RenderSession& session)
{
	// Do nothing -- Will be removed
}
} // namespace PR
