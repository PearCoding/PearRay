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
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "sampler/SampleArray.h"
#include "trace/IntersectionPoint.h"

#include "photon/PhotonMap.h"

#include "Logger.h"

/* Implementation of Stochastic Progressive Photon Mapping */

#define MIS(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

namespace PR {
struct PPMParameters {
	size_t MaxRayDepth		 = 64;
	size_t MaxPhotonsPerPass = 1000000;
	size_t MaxGatherCount	 = 100;
	float MaxGatherRadius	 = 10.0f;
	float SqueezeWeight2	 = 1.0f;
	float ContractRatio		 = 0.25f;
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
	std::shared_ptr<FrameBufferFloat> AccumulatedFlux;
	std::shared_ptr<FrameBufferFloat> SearchRadius2;
	std::shared_ptr<FrameBufferUInt32> LocalPhotonCount;

	inline PPMContext(float gridDelta)
		: Map(gridDelta)
	{
	}
};

enum GatherMode {
	GM_Sphere,
	GM_Dome
};

struct Contribution {
	float MIS;
	SpectralBlob Importance;
	SpectralBlob Radiance;
};
const static Contribution ZERO_CONTRIB = Contribution{ 0.0f, SpectralBlob::Zero(), SpectralBlob::Zero() };

constexpr float SHADOW_RAY_MIN = 0.0001f;
constexpr float SHADOW_RAY_MAX = PR_INF;
constexpr float BOUNCE_RAY_MIN = SHADOW_RAY_MIN;
constexpr float BOUNCE_RAY_MAX = PR_INF;

template <GatherMode GM>
class IntPPMInstance : public IIntegratorInstance {
public:
	explicit IntPPMInstance(PPMContext* context, const PPMParameters& parameters)
		: mContext(context)
		, mParameters(parameters)
	{
	}

	virtual ~IntPPMInstance() = default;

	// Estimate direct (infinite) light
	Contribution infiniteLight(RenderTileSession& session, const IntersectionPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Sample light
		InfiniteLightSampleInput inL;
		inL.Point = spt;
		inL.RND	  = session.tile()->random().get2D();
		InfiniteLightSampleOutput outL;
		infLight->sample(inL, outL, session);

		// An unlikely event, but a zero pdf has to be catched before blowing up other parts
		if (PR_UNLIKELY(outL.PDF_S <= PR_EPSILON))
			return ZERO_CONTRIB;

		if (infLight->hasDeltaDistribution())
			outL.PDF_S = 1;

		// Trace shadow ray
		const Ray shadow		= spt.Ray.next(spt.P, outL.Outgoing, spt.Surface.N, RF_Shadow, SHADOW_RAY_MIN, SHADOW_RAY_MAX);
		const bool hitSomething = session.traceOcclusionRay(shadow);

		SpectralBlob evalW;
		if (hitSomething) { // If we hit something before the light/background, the light path is occluded
			evalW = SpectralBlob::Zero();
		} else {
			evalW = outL.Radiance;
		}

		// Evaluate surface
		MaterialEvalInput in{ MaterialEvalContext::fromIP(spt, shadow.Direction), ShadingContext::fromIP(session.threadID(), spt) };
		MaterialEvalOutput out;
		material->eval(in, out, session);
		token = LightPathToken(out.Type);

		// Calculate hero mis pdf
		const float matPDF = (spt.Ray.Flags & RF_Monochrome) ? out.PDF_S[0] : (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE);
		const float msiL   = MIS(1, outL.PDF_S, 1, matPDF);
		return Contribution{ msiL, out.Weight / outL.PDF_S, evalW };
	}

	// Step one brdf
	void stepPath(RenderTileSession& session, LightPath& path,
				  IntersectionPoint& spt, SpectralBlob& importance,
				  IEntity*& entity, IMaterial*& material)
	{
		PR_PROFILE_THIS;

		if (spt.Ray.IterationDepth + 1 >= mParameters.MaxRayDepth) {
			material = nullptr;
			return;
		}

		// Sample BxDF
		MaterialSampleInput in{
			MaterialSampleContext::fromIP(spt),
			ShadingContext::fromIP(session.threadID(), spt),
			session.tile()->random().get2D()
		};
		MaterialSampleOutput out;
		material->sample(in, out, session);

		if (PR_UNLIKELY(out.PDF_S[0] <= PR_EPSILON)) {
			material = nullptr;
			return;
		}

		// Construct next ray
		Ray next = spt.Ray.next(spt.P, out.globalL(spt), spt.Surface.N, RF_Bounce, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
		if (material->isSpectralVarying())
			next.Flags |= RF_Monochrome;

		path.addToken(LightPathToken(out.Type)); // (1)

		importance *= out.Weight;

		// Trace bounce ray
		GeometryPoint npt;
		Vector3f npos;
		if (session.traceBounceRay(next, npos, npt, entity, material)) {
			session.tile()->statistics().addEntityHitCount();
			spt.setForSurface(next, npos, npt);
		} else {
			session.tile()->statistics().addBackgroundHitCount();
			material = nullptr;
			entity	 = nullptr;
		}
	}

	// Every camera vertex
	void walkPath(RenderTileSession& session, LightPath& path,
				  IntersectionPoint& spt, SpectralBlob& importance,
				  IEntity*& entity, IMaterial*& material)
	{
		// Early drop out for invalid splashes
		if (PR_UNLIKELY(!material))
			return;

		PR_PROFILE_THIS;

		while (material && material->hasDeltaDistribution()) {
			session.tile()->statistics().addDepthCount();
			stepPath(session, path, spt, importance, entity, material);
		}
	}

	// First camera vertex
	void handleCameraPath(RenderTileSession& session, LightPath& path,
						  const IntersectionPoint& spt,
						  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		// Early drop out for invalid splashes
		if (!entity->isLight() && PR_UNLIKELY(!material))
			return;

		session.pushSPFragment(spt, path);

		// Only consider camera rays, as everything else is handled eventually by MIS
		if (entity->isLight()) {
			IEmission* ems = session.getEmission(spt.Surface.Geometry.EmissionID);
			if (PR_LIKELY(ems)) {
				// Evaluate light
				LightEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);
				LightEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Weight.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), outL.Weight, spt.Ray, path);
					path.popToken();
				}
			}
		}

		IntersectionPoint actual_spt = spt;
		IEntity* actual_entity		 = entity;
		IMaterial* actual_material	 = material;
		SpectralBlob importance		 = SpectralBlob::Ones();

		walkPath(session, path, actual_spt, importance, actual_entity, actual_material);
		if (actual_material == nullptr) {
			if (actual_entity == nullptr) { // Background was hit
				path.addToken(LightPathToken::Background());
				for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
					InfiniteLightEvalInput lin;
					lin.Point = &actual_spt;
					lin.Ray	  = actual_spt.Ray;
					InfiniteLightEvalOutput lout;
					light->eval(lin, lout, session);

					if (lout.PDF_S <= PR_EPSILON)
						continue;

					session.pushSpectralFragment(SpectralBlob::Ones(), importance, lout.Radiance, actual_spt.Ray, path);
				}

				path.popToken();
			} else {
				// Error or max depth reached
				session.pushSpectralFragment(SpectralBlob::Ones(), importance, SpectralBlob::Zero(), actual_spt.Ray, path);
			}
		} else {
			// TODO: Gather

			// Infinite Lights
			// 1 sample per inf-light
			for (auto light : session.tile()->context()->scene()->infiniteLights()) {
				LightPathToken token;
				const auto contrib = infiniteLight(session, actual_spt, token, light.get(), actual_material);

				if (contrib.MIS <= PR_EPSILON)
					continue;

				path.addToken(token);
				path.addToken(LightPathToken::Background());
				session.pushSpectralFragment(SpectralBlob(contrib.MIS), importance * contrib.Importance, contrib.Radiance, spt.Ray, path);
				path.popToken(2);
			}
		}
		path.popTokenUntil(1);
	}

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const size_t maxPathSize = mParameters.MaxRayDepth + 2;

		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());

		session.tile()->statistics().addDepthCount(sg.size());
		session.tile()->statistics().addEntityHitCount(sg.size());

		for (size_t i = 0; i < sg.size(); ++i) {
			IntersectionPoint spt;
			sg.computeShadingPoint(i, spt);

			handleCameraPath(session, path, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
		}
	}

	void handleBackground(RenderTileSession& session, const ShadingGroup& sg)
	{ // Same as direct integrator
		PR_PROFILE_THIS;
		LightPath cb = LightPath::createCB();
		session.tile()->statistics().addDepthCount(sg.size());
		session.tile()->statistics().addBackgroundHitCount(sg.size());

		if (!session.tile()->context()->scene()->nonDeltaInfiniteLights().empty()) {
			for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
				for (size_t i = 0; i < sg.size(); ++i) {
					InfiniteLightEvalInput in;
					sg.extractRay(i, in.Ray);
					in.Point = nullptr;
					InfiniteLightEvalOutput out;
					light->eval(in, out, session);

					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), out.Radiance, in.Ray, cb);
				}
			}
		} else { // If no inf. lights are available make sure at least zero is splatted
			for (size_t i = 0; i < sg.size(); ++i) {
				Ray ray;
				sg.extractRay(i, ray);
				session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), SpectralBlob::Zero(), ray, cb);
			}
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
					handleBackground(session, sg);
				else
					handleShadingGroup(session, sg);
			}
		}
	}

	void photonPass(RenderTileSession& session)
	{
		PR_PROFILE_THIS;

		auto& threadData = mContext->Threads[session.threadID()];
		if (threadData.Handled)
			return;
		threadData.Handled = true;

		for (const auto& light : threadData.Lights) {
			const float sampleInv = 1.0f / light.Photons;

			// Cache
			uint32 lastEmissionID = PR_INVALID_ID;
			IEmission* emission	  = nullptr;

			size_t photonsShoot	 = 0;
			size_t photonsStored = 0;
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
				const Vector3f local_dir = Sampling::hemi(drnd[0], drnd[1]);
				const Vector3f dir		 = Tangent::fromTangentSpace(gp.N, gp.Nx, gp.Ny, local_dir);

				Ray ray = Ray(pp.Position, dir);
				ray.Flags |= RF_Light;

				if (PR_UNLIKELY(gp.EmissionID != lastEmissionID)) {
					lastEmissionID = gp.EmissionID;
					emission	   = session.getEmission(gp.EmissionID);
				}

				LightEvalInput in;
				in.Entity					   = light.Entity;
				in.ShadingContext.Face		   = gp.PrimitiveID;
				in.ShadingContext.dUV		   = gp.dUV;
				in.ShadingContext.UV		   = gp.UV;
				in.ShadingContext.ThreadIndex  = session.threadID();
				in.ShadingContext.WavelengthNM = SpectralBlob(550, 520, 460, 620); // TODO

				LightEvalOutput out;
				emission->eval(in, out, session);
				out.Weight /= pp.PDF.Value * Sampling::hemi_pdf();

				for (size_t j = 0; j < mParameters.MaxRayDepth; ++j) {
					Vector3f pos;
					GeometryPoint ngp;
					IEntity* entity		= nullptr;
					IMaterial* material = nullptr;
					if (!session.traceBounceRay(ray, pos, ngp, entity, material))
						break;

					if (entity && material && material->canBeShaded()) {
						IntersectionPoint ip;
						ip.setForSurface(ray, pos, ngp);

						MaterialSampleInput sin;
						sin.Context		   = MaterialSampleContext::fromIP(ip);
						sin.ShadingContext = ShadingContext::fromIP(session.threadID(), ip);

						MaterialSampleOutput sout;
						material->sample(sin, sout, session);

						if (material->hasDeltaDistribution()) {
							// Continue
							out.Weight *= SpectralBlobUtils::HeroOnly() * sout.Weight / sout.PDF_S;
							ray = ray.next(ip.P, sout.globalL(ip), ip.Surface.N,
										   RF_Monochrome | RF_Light, BOUNCE_RAY_MIN, BOUNCE_RAY_MAX);
						} else {
							// Always store when diffuse
							Photon::Photon photon;
							for (int k = 0; k < 3; ++k) {
								photon.Position[k]	= ip.P(k);
								photon.Direction[k] = -ray.Direction(k);
							}

							photon.Power	   = sout.Weight * sampleInv;
							photon.Wavelengths = sin.ShadingContext.WavelengthNM;
							mContext->Map.store(photon);

							photonsStored++;
						}

					} else { // Nothing found, abort
						break;
					}
				}
			}
		}
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
			mContext = std::make_unique<PPMContext>(mParameters.MaxGatherRadius);
			setupContext(renderer);
		}
		mThreadMutex.unlock();
		return std::make_shared<IntPPMInstance<GM>>(mContext.get(), mParameters);
	}

private:
	void setupContext(RenderContext* renderer)
	{
		std::unordered_map<IEntity*, PPMLightCache> lights;
		assignLights(renderer, lights);
		assignTiles(renderer, lights);

		// Make sure the photon map is always cleared before photon pass
		renderer->addIterationCallback([this](uint32 iter) {
			if (iter % 2 == 0) { // Photon pass
				mContext->Map.reset();
				for (auto& td : mContext->Threads)
					td.Handled = false;
			}
		});
		// TODO: What if the integrator context gets destroyed?

		// Create framebuffers
		// Notice: There is no need for tile local framebuffers as no filters are used and random pixel access is prohibited
		const auto output		   = renderer->output()->data().getInternalChannel_Spectral(AOV_Output);
		mContext->AccumulatedFlux  = std::make_shared<FrameBufferFloat>(output->channels(), output->size());
		mContext->SearchRadius2	   = std::make_shared<FrameBufferFloat>(1, output->size());
		mContext->LocalPhotonCount = std::make_shared<FrameBufferUInt32>(1, output->size());

		mContext->AccumulatedFlux->fill(0);
		mContext->SearchRadius2->fill(mParameters.MaxGatherRadius * mParameters.MaxGatherRadius);
		mContext->LocalPhotonCount->fill(0);
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
		mParameters.MaxPhotonsPerPass = std::max<size_t>(100, params.getUInt("photons", mParameters.MaxPhotonsPerPass));
		mParameters.MaxRayDepth		  = (size_t)params.getUInt("max_ray_depth", mParameters.MaxRayDepth);
		mParameters.MaxGatherCount	  = std::max<size_t>(100, params.getUInt("max_gather_count", mParameters.MaxGatherCount));
		mParameters.MaxGatherRadius	  = std::max(0.00001f, params.getNumber("max_gather_radius", mParameters.MaxGatherRadius));

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
		const static std::vector<std::string> names({ "ppm", "sppm", "photon" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntPPMPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)