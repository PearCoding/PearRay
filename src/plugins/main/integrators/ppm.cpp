#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "buffer/Feedback.h"
#include "buffer/FrameBufferSystem.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "light/LightSampler.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "math/Sampling.h"
#include "path/LightPath.h"
#include "photon/PhotonMap.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "sampler/SampleArray.h"

#include "IntegratorUtils.h"
#include "Walker.h"

#include "Logger.h"

/* Implementation of Propabilistic Progressive Photon Mapping */

namespace PR {
constexpr float ACCUM_EPS = 0.00001f;

static inline float culling(float cos)
{
#ifdef PR_NO_CULLING
	return std::abs(cos);
#else
	return std::max(0.0f, cos);
#endif
}

struct PPMParameters {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
	size_t MaxPhotonsPerPass	 = 1000000;
	float MaxGatherRadius		 = 0.1f;
	float SqueezeWeight2		 = 0.0f;
	float ContractRatio			 = 0.4f;
};

struct PPMLightCache {
	const PR::Light* Light;
	uint64 Photons;
};

struct PPMThreadData {
	std::vector<PPMLightCache> Lights;
	bool Handled = false; // Will make sure it will be handled once per iteration
};

struct PPMContext {
	Photon::PhotonMap Map;
	std::vector<PPMThreadData> Threads;
	float CurrentSearchRadius2 = 0.0f;
	float CurrentKernelInvNorm = 0.0f;

	inline PPMContext(const BoundingBox& bbox, float gridDelta)
		: Map(bbox, gridDelta)
	{
	}
};

enum GatherMode {
	GM_Sphere,
	GM_Dome
};

// TODO
constexpr float WAVELENGTH_START = 360;
constexpr float WAVELENGTH_END	 = 760;
constexpr float WAVEBAND		 = 5;
inline static SpectralBlob sampleWavelength(Random& rnd)
{
	constexpr float span  = WAVELENGTH_END - WAVELENGTH_START;
	constexpr float delta = span / PR_SPECTRAL_BLOB_SIZE;

	SpectralBlob wvls;
	const float start = rnd.getFloat() * span;	  // Wavelength inside the span
	wvls(0)			  = start + WAVELENGTH_START; // Hero wavelength
	PR_OPT_LOOP
	for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		wvls(i) = WAVELENGTH_START + std::fmod(start + i * delta, span);
	return wvls;
}

// TODO
inline static float wavelengthFilter(float a, float b)
{
	constexpr float PDF = 1 - 0.5f * WAVEBAND / (WAVELENGTH_END - WAVELENGTH_START); // Area of box[-w/2,w/2] x box[0,1] convolution
	const bool check	= std::abs(a - b) <= (WAVEBAND / 2);
	return check ? 1.0f / PDF : 0.0f;
}

// Integral over area of circle with radius r
/*inline static float kernel(float nr2) { return 1 - std::sqrt(nr2); }
inline static float kernelarea(float R2) { return PR_PI * R2 / 3.0f; }*/

inline static float kernel(float nr2) { return 1 - nr2; }
inline static float kernelarea(float R2) { return PR_PI * R2 / 2.0f; }

using LightPathWalker  = Walker<true>;	// Enable russian roulette
using CameraPathWalker = Walker<false>; // No russian roulette needed, as only delta materials scatter
template <GatherMode GM>
class IntPPMInstance : public IIntegratorInstance {
public:
	explicit IntPPMInstance(PPMContext* context,
							const std::shared_ptr<LightSampler>& lightSampler, const PPMParameters& parameters)
		: mContext(context)
		, mParameters(parameters)
		, mLightSampler(lightSampler)
	{
		mLightPathWalker.MaxRayDepthHard  = mParameters.MaxLightRayDepthHard;
		mLightPathWalker.MaxRayDepthSoft  = mParameters.MaxLightRayDepthSoft;
		mCameraPathWalker.MaxRayDepthHard = mParameters.MaxCameraRayDepthHard;
		mCameraPathWalker.MaxRayDepthSoft = mParameters.MaxCameraRayDepthSoft; // Obsolete
	}

	virtual ~IntPPMInstance() = default;

	SpectralBlob gather(const RenderTileSession& session, const IntersectionPoint& spt, IMaterial* material)
	{
		PR_UNUSED(material);
		PR_UNUSED(session);
		Photon::PhotonSphere query;

		query.SqueezeWeight = mParameters.SqueezeWeight2;
		query.Center		= spt.P;
		query.Normal		= spt.Surface.N;
		query.Distance2		= mContext->CurrentSearchRadius2;

		MaterialEvalInput min;
		min.Context		   = MaterialEvalContext::fromIP(spt, Vector3f(0, 0, 0));
		min.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);

		const auto accumFunc = [&](SpectralBlob& accum, const Photon::Photon& photon, const Photon::PhotonSphere& sp, float d2) {
			PR_UNUSED(sp);

			const float f = kernel(d2 / query.Distance2);

			const SpectralBlob power = photon.Power * f;
			SpectralBlob weight		 = SpectralBlob::Zero();
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				for (size_t j = 0; j < PR_SPECTRAL_BLOB_SIZE; ++j)
					weight[i] += wavelengthFilter(spt.Ray.WavelengthNM[i], photon.WavelengthNM[j]) * power[j];

			if (weight.isZero(ACCUM_EPS))
				return;

			min.Context.setLFromGlobal(spt, photon.Direction);
			MaterialEvalOutput mout;
			material->eval(min, mout, session);
			accum += mout.Weight * weight;
		};

		size_t found = 0;
		SpectralBlob accum;
		if constexpr (GM == GM_Dome)
			accum = mContext->Map.estimateDome(query, accumFunc, found);
		else
			accum = mContext->Map.estimateSphere(query, accumFunc, found);

		accum *= mContext->CurrentKernelInvNorm;

		return accum;
	}

	// First camera vertex
	void handleCameraPath(RenderTileSession& session, LightPath& path,
						  const IntersectionPoint& spt,
						  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		session.pushSPFragment(spt, path);

		mCameraPathWalker.traverseBSDF(
			session, SpectralBlob::Ones(), spt, entity, material,
			[&](const SpectralBlob& weight, const IntersectionPoint& ip, IEntity* entity_hit, IMaterial* material_hit) {
				session.tile()->statistics().addEntityHitCount();
				session.tile()->statistics().addCameraDepthCount();

				// If we hit a light evaluate it and stop
				if (entity_hit->hasEmission()
					&& culling(-ip.Surface.NdotV) > PR_EPSILON) // Check if frontside
				{
					const IEmission* ems = session.getEmission(ip.Surface.Geometry.EmissionID);
					if (PR_LIKELY(ems)) {
						// Evaluate light
						EmissionEvalInput inL;
						inL.Entity		   = entity_hit;
						inL.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);
						EmissionEvalOutput outL;
						ems->eval(inL, outL, session);

						path.addToken(LightPathToken::Emissive());
						session.pushSpectralFragment(SpectralBlob::Ones(), weight, outL.Radiance, ip.Ray, path);
						path.popToken();
					}
					return false;
				}

				if (!material_hit->hasDeltaDistribution()) {
					const auto gather_contrib = gather(session, ip, material_hit);
					path.addToken(LightPathToken::Emissive());
					session.pushSpectralFragment(SpectralBlob::Ones(),
												 weight, gather_contrib,
												 ip.Ray, path);
					path.popToken(1);
					return false;
				} else {
					return true; // Only traverse for delta materials
				}
			},
			[&](SpectralBlob& weight, const MaterialSampleInput&, const MaterialSampleOutput& sout, IEntity*, IMaterial*) {
				weight *= sout.Weight;
				path.addToken(sout.Type);
			},
			[&](const SpectralBlob& weight, const Ray& ray) {
				path.addToken(LightPathToken::Background());

				const bool illuminated = IntegratorUtils::handleBackground(session, ray, [&](const InfiniteLightEvalOutput& ileout) {
					session.pushSpectralFragment(SpectralBlob::Ones(), weight, ileout.Radiance, ray, path);
				});

				if (!illuminated) {
					session.tile()->statistics().addBackgroundHitCount();
					session.pushSpectralFragment(SpectralBlob::Ones(), weight, SpectralBlob::Zero(), ray, path);
				}
				
				path.popToken();
			});
		path.popTokenUntil(1);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const size_t maxPathSize = mParameters.MaxCameraRayDepthHard + 2;

		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			handleCameraPath(session, path, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
		}
	}

	void accumPass(RenderTileSession& session)
	{
		PR_PROFILE_THIS;
		while (!session.pipeline()->isFinished()) {
			session.pipeline()->runPipeline();
			while (session.pipeline()->hasShadingGroup()) {
				auto sg = session.pipeline()->popShadingGroup(session);
				if (sg.isBackground())
					IntegratorUtils::handleBackgroundGroup(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

	void emitPhotons(RenderTileSession& session, const PPMLightCache& light)
	{
		// The actual photon count is the result of the multiplication with the hero wavelength component count
		const float sampleInv = 1.0f / (PR_SPECTRAL_BLOB_SIZE * light.Photons);

		size_t photonsShoot = 0;
		for (; photonsShoot < light.Photons; ++photonsShoot) {
			LightSampleInput lsin;
			lsin.WavelengthNM	= sampleWavelength(session.random());
			lsin.RND			= session.random().get4D();
			lsin.SamplePosition = true;
			LightSampleOutput lsout;
			light.Light->sample(lsin, lsout, session);

			Ray ray	   = Ray(lsout.LightPosition, -lsout.Outgoing);
			ray.Origin = Transform::safePosition(ray.Origin, ray.Direction); // Make sure no self intersection happens
			ray.Flags |= RF_Light;
			ray.WavelengthNM = lsin.WavelengthNM;

			const float pdf				= lsout.Direction_PDF_S * lsout.Position_PDF.Value;
			const SpectralBlob radiance = lsout.Radiance * (sampleInv / pdf);

			mLightPathWalker.traverseBSDFSimple(
				session, radiance, ray,
				[&](const SpectralBlob& weight, const IntersectionPoint& ip, IEntity* entity, IMaterial* material) {
					session.tile()->statistics().addEntityHitCount();
					session.tile()->statistics().addLightDepthCount();

					if (entity->hasEmission()) // Stop at lights and do not save photons
						return false;

					if (!material->hasDeltaDistribution()) { // Always store when diffuse
						Photon::Photon photon;
						photon.Position		= ip.P;
						photon.Direction	= -ray.Direction;
						photon.Power		= weight;
						photon.WavelengthNM = ray.WavelengthNM;

						if (ray.Flags & RF_Monochrome) { // If monochrome, spread hero to each channel
							photon.Power		= photon.Power[0] / PR_SPECTRAL_BLOB_SIZE;
							photon.WavelengthNM = photon.WavelengthNM[0];
						}

						mContext->Map.storeUnsafe(photon);
					}
					return true;
				},
				[](const SpectralBlob&, const Ray&) {} /* Ignore non hits */);
		}
	}

	void photonPass(RenderTileSession& session)
	{
		PR_PROFILE_THIS;

		auto& threadData = mContext->Threads[session.threadID()];
		if (threadData.Handled)
			return;
		threadData.Handled = true;

		// TODO: Use wavefront approach!
		for (const auto& light : threadData.Lights)
			emitPhotons(session, light);
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;
		const bool isAccumPass = session.context()->currentIteration().Pass != 0;

		if (!isAccumPass)
			photonPass(session);
		else
			accumPass(session);
	}

private:
	PPMContext* mContext;
	const PPMParameters mParameters;
	const std::shared_ptr<LightSampler> mLightSampler;

	LightPathWalker mLightPathWalker;
	CameraPathWalker mCameraPathWalker;
};

template <GatherMode GM>
class IntPPM : public IIntegrator {
public:
	explicit IntPPM(const PPMParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntPPM() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* renderer, size_t) override
	{
		mThreadMutex.lock();
		if (!mContext) {
			const float edge = std::max(0.1f, renderer->scene()->boundingBox().shortestEdge());
			mContext		 = std::make_unique<PPMContext>(renderer->scene()->boundingBox(), edge / 100);
			setupContext(renderer);
		}
		mThreadMutex.unlock();
		return std::make_shared<IntPPMInstance<GM>>(mContext.get(), renderer->lightSampler(), mParameters);
	}

	IntegratorConfiguration configuration() const override { return IntegratorConfiguration{ 2 /*Pass Count*/ }; }

private:
	void beforePhotonPass(uint32 photonPass)
	{
		mContext->Map.reset();
		for (auto& td : mContext->Threads)
			td.Handled = false;

		if (photonPass == 0)
			mContext->CurrentSearchRadius2 = mParameters.MaxGatherRadius * mParameters.MaxGatherRadius;
		else
			mContext->CurrentSearchRadius2 *= (photonPass + 1 + mParameters.ContractRatio) / (photonPass + 2);

		mContext->CurrentSearchRadius2 = std::max(mContext->CurrentSearchRadius2, 0.00001f);
		PR_LOG(L_DEBUG) << "PPM Radius2=" << mContext->CurrentSearchRadius2 << std::endl;

		mContext->CurrentKernelInvNorm = 1.0f / kernelarea(mContext->CurrentSearchRadius2);
	}

	using LightMap = std::unordered_map<const Light*, PPMLightCache>;
	void setupContext(RenderContext* renderer)
	{
		LightMap lights;
		assignLights(renderer, lights);
		assignTiles(renderer, lights);
		LightMap().swap(lights); // Delete

		// Make sure the photon map is always cleared before photon pass
		renderer->addIterationCallback([this](const RenderIteration& iter) {
			if (iter.Pass == 0)
				beforePhotonPass(iter.Iteration);
		});
		// TODO: What if the integrator context gets destroyed?
	}

	void assignLights(RenderContext* renderer, LightMap& lights)
	{
		const uint64 Photons	= mParameters.MaxPhotonsPerPass;
		const uint64 MinPhotons = Photons * 0.02f; // Should be a parameter
		const auto lightSampler = renderer->lightSampler();
		const auto& lightList	= lightSampler->lights();

		const uint64 k = MinPhotons * lightList.size();
		if (k >= Photons) { // Not enough photons given.
			PR_LOG(L_WARNING) << "Not enough photons per pass given. At least " << k << " is needed." << std::endl;

			for (const auto& light : lightList)
				lights[light.get()] = PPMLightCache{ light.get(), MinPhotons };
		} else {
			const uint64 d = Photons - k;

			// Assign photons based on relative contribution to the scene
			PR_LOG(L_DEBUG) << "PPM Lights" << std::endl;
			for (const auto& light : lightList) {
				PPMLightCache cache;
				cache.Light	  = light.get();
				cache.Photons = MinPhotons + std::ceil(d * cache.Light->relativeContribution());

				PR_LOG(L_DEBUG) << "  -> Light '" << light->name() << "' " << cache.Photons << " photons "
								<< cache.Light->relativeContribution() * 100 << "%" << std::endl;
				lights[cache.Light] = std::move(cache);
			}
		}
	}

	void assignTiles(RenderContext* renderer, const LightMap& lights)
	{
		const uint64 Photons		  = mParameters.MaxPhotonsPerPass;
		const uint64 PhotonsPerThread = std::ceil(Photons / static_cast<float>(renderer->threadCount()));
		PR_LOG(L_DEBUG) << "Each thread will shoot " << PhotonsPerThread << " photons" << std::endl;

		// Spread light photon pair to tiles
		std::vector<uint64> photonsPerThread(renderer->threadCount(), 0);
		mContext->Threads.resize(photonsPerThread.size());
		for (const auto& light : lights) {
			uint64 photonsSpread = 0;
			for (size_t t = 0; t < renderer->threadCount(); ++t) {
				if (photonsSpread >= light.second.Photons)
					break;

				if (photonsPerThread[t] >= PhotonsPerThread)
					continue;

				const uint64 photons = std::min(PhotonsPerThread - photonsPerThread[t],
												light.second.Photons - photonsSpread);

				photonsSpread += photons;
				photonsPerThread[t] += photons;

				if (photons > 0)
					mContext->Threads[t].Lights.push_back(PPMLightCache{ light.second.Light, photons });
			}
		}

		// Debug output
		for (size_t t = 0; t < renderer->threadCount(); ++t) {
			PR_LOG(L_DEBUG) << "PPM Thread has " << mContext->Threads[t].Lights.size() << " lights" << std::endl;
			for (const auto& ltd : mContext->Threads[t].Lights)
				PR_LOG(L_DEBUG) << "  -> Light '" << ltd.Light->name() << "' with " << ltd.Photons << " photons" << std::endl;
		}
	}

	const PPMParameters mParameters;
	std::mutex mThreadMutex;
	std::unique_ptr<PPMContext> mContext;
};

class IntPPMFactory : public IIntegratorFactory {
public:
	explicit IntPPMFactory(const ParameterGroup& params)
	{
		mParameters.MaxPhotonsPerPass = std::max<size_t>(100, params.getUInt("photons", mParameters.MaxPhotonsPerPass));

		size_t maximumDepth = std::numeric_limits<size_t>::max();
		if (params.hasParameter("max_ray_depth"))
			maximumDepth = params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);

		mParameters.MaxCameraRayDepthHard = std::min<size_t>(maximumDepth, params.getUInt("max_camera_ray_depth", mParameters.MaxCameraRayDepthHard));
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_camera_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = std::min<size_t>(maximumDepth, params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard));
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));

		mParameters.MaxGatherRadius = std::max(0.00001f, params.getNumber("max_gather_radius", mParameters.MaxGatherRadius));

		mParameters.SqueezeWeight2 = std::max(0.0f, std::min(1.0f, params.getNumber("squeeze_weight", mParameters.SqueezeWeight2)));
		mParameters.SqueezeWeight2 *= mParameters.SqueezeWeight2;

		mParameters.ContractRatio = std::max(0.0f, std::min(1.0f, params.getNumber("contract_ratio", mParameters.ContractRatio)));

		std::string mode = params.getString("gather_mode", "dome");
		std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
		if (mode == "dome")
			mGatherMode = GM_Dome;
		else
			mGatherMode = GM_Sphere;
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		switch (mGatherMode) {
		case GM_Dome:
			return std::make_shared<IntPPM<GM_Dome>>(mParameters);
		case GM_Sphere:
		default:
			return std::make_shared<IntPPM<GM_Sphere>>(mParameters);
		}
	}

private:
	PPMParameters mParameters;
	GatherMode mGatherMode;
};

class IntPPMPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntPPMFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "ppm", "sppm", "pppm", "photon" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntPPMPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)