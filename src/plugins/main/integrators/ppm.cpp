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
#include "trace/IntersectionPoint.h"

#include "IntegratorUtils.h"
#include "Walker.h"

#include "Logger.h"

/* Implementation of Propabilistic Progressive Photon Mapping */

namespace PR {
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
	IEntity* Entity;
	float SurfaceArea;
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
constexpr float WAVELENGTH_START = 400;
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

inline static SpectralBlob wavelengthFilter(const SpectralBlob& a, const SpectralBlob& b)
{
	constexpr float PDF = 1 - 0.25f * WAVEBAND / (WAVELENGTH_END - WAVELENGTH_START); // Area of box[-w/2,w/2] x box[0,1] convolution

	return ((a - b).cwiseAbs() <= (WAVEBAND / 2))
		.select(SpectralBlob(1 / PDF),
				SpectralBlob::Zero());
}

// Integral over area of circle with radius r
/*inline static float kernel(float nr2) { return 1 - std::sqrt(nr2); }
inline static float kernelarea(float R2) { return PR_PI * R2 / 3.0f; }*/

inline static float kernel(float nr2) { return 1 - nr2; }
inline static float kernelarea(float R2) { return PR_PI * R2 / 2.0f; }

struct PPMContribution {
	SpectralBlob Importance;
	SpectralBlob Radiance;
};
using Contribution					   = PPMContribution;
const static Contribution ZERO_CONTRIB = Contribution{ SpectralBlob::Zero(), SpectralBlob::Zero() };

using LightPathWalker  = Walker<true>;	// Enable russian roulette
using CameraPathWalker = Walker<false>; // No russian roulette needed, as only delta materials scatter
template <GatherMode GM>
class IntPPMInstance : public IIntegratorInstance {
public:
	explicit IntPPMInstance(PPMContext* context, const PPMParameters& parameters)
		: mContext(context)
		, mParameters(parameters)
	{
		mLightPathWalker.MaxRayDepthHard  = mParameters.MaxLightRayDepthHard;
		mLightPathWalker.MaxRayDepthSoft  = mParameters.MaxLightRayDepthSoft;
		mCameraPathWalker.MaxRayDepthHard = mParameters.MaxCameraRayDepthHard;
		mCameraPathWalker.MaxRayDepthSoft = mParameters.MaxCameraRayDepthSoft; // Obsolete
	}

	virtual ~IntPPMInstance() = default;

	// Estimate direct (infinite) light
	Contribution infiniteLight(RenderTileSession& session, const IntersectionPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material)
	{
		PR_PROFILE_THIS;

		auto sample = IntegratorUtils::sampleInfiniteLight(session, spt, infLight);
		if (!sample.has_value())
			return ZERO_CONTRIB;
		else {
			// Evaluate surface
			MaterialEvalInput in{ MaterialEvalContext::fromIP(spt, sample.value().Direction), ShadingContext::fromIP(session.threadID(), spt) };
			MaterialEvalOutput out;
			material->eval(in, out, session);
			token = LightPathToken(out.Type);

			return Contribution{ out.Weight / sample.value().PDF_S, sample.value().Weight };
		}
	}

	Contribution gather(const RenderTileSession& session, const IntersectionPoint& spt, IMaterial* material)
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

			min.Context.setLFromGlobal(spt, photon.Direction);
			MaterialEvalOutput mout;
			material->eval(min, mout, session);
			const float f = kernel(d2 / query.Distance2);

			accum += wavelengthFilter(spt.Ray.WavelengthNM, photon.WavelengthNM) * photon.Power * mout.Weight * f;
		};

		size_t found = 0;
		SpectralBlob accum;
		if constexpr (GM == GM_Dome)
			accum = mContext->Map.estimateDome(query, accumFunc, found);
		else
			accum = mContext->Map.estimateSphere(query, accumFunc, found);

		//if (found >= 1) {
		accum *= mContext->CurrentKernelInvNorm;
		// TODO: Add PPM and other parts together!
		return Contribution{ SpectralBlob::Ones(), accum };
		/*} else {
			return ZERO_CONTRIB;
		}*/
	}

	// First camera vertex
	void handleCameraPath(RenderTileSession& session, LightPath& path,
						  const IntersectionPoint& spt,
						  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Early drop out for invalid splashes
		if (!entity->hasEmission() && PR_UNLIKELY(!material))
			return;

		session.pushSPFragment(spt, path);

		// Only consider camera rays, as everything else is handled eventually by MIS
		if (entity->hasEmission()) {
			IEmission* ems = session.getEmission(spt.Surface.Geometry.EmissionID);
			if (PR_LIKELY(ems)) {
				// Evaluate light
				EmissionEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);
				EmissionEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Radiance.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), outL.Radiance, spt.Ray, path);
					path.popToken();
				}
			}
		}

		mCameraPathWalker.traverseBSDF(
			session, SpectralBlob::Ones(), spt, entity, material,
			[&](const SpectralBlob& weight, const IntersectionPoint& ip, IEntity*, IMaterial* material_hit) {
				session.tile()->statistics().addEntityHitCount();
				session.tile()->statistics().addDepthCount();

				if (!material_hit->hasDeltaDistribution()) {
					const auto gather_contrib = gather(session, ip, material_hit);
					path.addToken(LightPathToken::Emissive());
					session.pushSpectralFragment(SpectralBlob::Ones(),
												 weight * gather_contrib.Importance, gather_contrib.Radiance,
												 ip.Ray, path);
					path.popToken(1);

					// Infinite Lights
					// 1 sample per inf-light
					for (auto light : session.tile()->context()->scene()->infiniteLights()) {
						LightPathToken token;
						const auto contrib = infiniteLight(session, ip, token, light.get(), material_hit);

						path.addToken(token);
						path.addToken(LightPathToken::Background());
						session.pushSpectralFragment(SpectralBlob::Ones(), weight * contrib.Importance, contrib.Radiance, ip.Ray, path);
						path.popToken(2);
					}
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
				IntegratorUtils::handleBackground(session, path, weight, ray);
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

		// Cache
		uint32 lastEmissionID = PR_INVALID_ID;
		IEmission* emission	  = nullptr;

		size_t photonsShoot = 0;
		for (; photonsShoot < light.Photons; ++photonsShoot) {
			// Randomly select point on light surface
			const auto pp = light.Entity->sampleParameterPoint(session.tile()->random().get2D());

			EntityGeometryQueryPoint qp;
			qp.Position	   = pp.Position;
			qp.UV		   = pp.UV;
			qp.PrimitiveID = pp.PrimitiveID;
			qp.View		   = Vector3f::Zero();

			GeometryPoint gp;
			light.Entity->provideGeometryPoint(qp, gp);

			// Randomly sample direction from emissive material (TODO: Should be abstracted)
			const Vector2f drnd		 = session.tile()->random().get2D();
			const Vector3f local_dir = Sampling::cos_hemi(drnd[0], drnd[1]);
			const Vector3f dir		 = Tangent::fromTangentSpace(gp.N, gp.Nx, gp.Ny, local_dir);

			Ray ray = Ray(pp.Position, dir);
			ray.Flags |= RF_Light;
			ray.WavelengthNM = sampleWavelength(session.tile()->random());

			if (PR_UNLIKELY(gp.EmissionID != lastEmissionID)) {
				lastEmissionID = gp.EmissionID;
				emission	   = session.getEmission(gp.EmissionID);
			}

			EmissionEvalInput in;
			in.Entity					   = light.Entity;
			in.ShadingContext.Face		   = gp.PrimitiveID;
			in.ShadingContext.dUV		   = gp.dUV;
			in.ShadingContext.UV		   = gp.UV;
			in.ShadingContext.ThreadIndex  = session.threadID();
			in.ShadingContext.WavelengthNM = ray.WavelengthNM;

			EmissionEvalOutput out;
			emission->eval(in, out, session);
			out.Radiance *= sampleInv / (pp.PDF.Value * Sampling::cos_hemi_pdf(local_dir(2)));

			mLightPathWalker.traverseBSDFSimple(
				session, out.Radiance, ray,
				[&](const SpectralBlob& weight, const IntersectionPoint& ip, IEntity* entity, IMaterial* material) {
					session.tile()->statistics().addEntityHitCount();
					session.tile()->statistics().addDepthCount();

					PR_UNUSED(entity);
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

		const bool isAccumPass = session.tile()->iterationCount() % 2 != 0;

		if (!isAccumPass)
			photonPass(session);
		else
			accumPass(session);
	}

private:
	PPMContext* mContext;
	const PPMParameters mParameters;

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
		return std::make_shared<IntPPMInstance<GM>>(mContext.get(), mParameters);
	}

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

	void setupContext(RenderContext* renderer)
	{
		std::unordered_map<IEntity*, PPMLightCache> lights;
		assignLights(renderer, lights);
		assignTiles(renderer, lights);

		// Make sure the photon map is always cleared before photon pass
		renderer->addIterationCallback([this](uint32 iter) {
			if (iter % 2 == 0)
				beforePhotonPass(iter / 2);
		});
		// TODO: What if the integrator context gets destroyed?
	}

	void assignLights(RenderContext* renderer, std::unordered_map<IEntity*, PPMLightCache>& lights)
	{
		const uint64 Photons	= mParameters.MaxPhotonsPerPass;
		const uint64 MinPhotons = Photons * 0.02f; // Should be a parameter
		const auto& lightList	= renderer->lights();

		const uint64 k = MinPhotons * lightList.size();
		if (k >= Photons) { // Not enough photons given.
			PR_LOG(L_WARNING) << "Not enough photons per pass given. At least " << k << " is needed." << std::endl;

			for (const auto& light : lightList)
				lights[light.get()] = PPMLightCache{ light.get(), light->worldSurfaceArea(), MinPhotons };
		} else {
			const uint64 d = Photons - k;

			// Extract surface area information
			float fullArea = 0;
			for (const auto& light : lightList) {
				PPMLightCache cache;
				cache.Entity	  = light.get();
				cache.SurfaceArea = light->worldSurfaceArea();
				fullArea += cache.SurfaceArea;
				lights[light.get()] = std::move(cache);
			}

			// Assign photons based on surface
			PR_LOG(L_DEBUG) << "PPM Lights" << std::endl;
			for (const auto& light : lightList) {
				PPMLightCache& cache = lights[light.get()];
				cache.Photons		 = MinPhotons + std::ceil(d * (cache.SurfaceArea / fullArea));

				PR_LOG(L_DEBUG) << "  -> Light '" << light->name() << "' " << cache.Photons << " photons " << cache.SurfaceArea << "m2" << std::endl;
			}
		}
	}

	void assignTiles(RenderContext* renderer, const std::unordered_map<IEntity*, PPMLightCache>& lights)
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
					mContext->Threads[t].Lights.push_back(PPMLightCache{ light.second.Entity, light.second.SurfaceArea, photons });
			}
		}

		// Debug output
		for (size_t t = 0; t < renderer->threadCount(); ++t) {
			PR_LOG(L_DEBUG) << "PPM Thread has " << mContext->Threads[t].Lights.size() << " lights" << std::endl;
			for (const auto& ltd : mContext->Threads[t].Lights)
				PR_LOG(L_DEBUG) << "  -> Light '" << ltd.Entity->name() << "' with " << ltd.Photons << " photons" << std::endl;
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
		mParameters.MaxPhotonsPerPass	  = std::max<size_t>(100, params.getUInt("photons", mParameters.MaxPhotonsPerPass));
		mParameters.MaxCameraRayDepthHard = (size_t)params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = (size_t)params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard);
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));
		mParameters.MaxGatherRadius		  = std::max(0.00001f, params.getNumber("max_gather_radius", mParameters.MaxGatherRadius));

		mParameters.SqueezeWeight2 = std::max(0.0f, std::min(1.0f, params.getNumber("squeeze_weight", mParameters.SqueezeWeight2)));
		mParameters.SqueezeWeight2 *= mParameters.SqueezeWeight2;

		mParameters.ContractRatio = std::max(0.0f, std::min(1.0f, params.getNumber("contract_ratio", mParameters.ContractRatio)));

		std::string mode = params.getString("gather_mode", "sphere");
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
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
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