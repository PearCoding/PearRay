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

/* Implementation of a bidirectional path tracer */

#define MIS(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

namespace PR {
struct BiDiParameters {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
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

struct BiDiContribution {
	float MIS;
	SpectralBlob Importance;
	SpectralBlob Radiance;
};
using Contribution					   = BiDiContribution;
const static Contribution ZERO_CONTRIB = Contribution{ 0.0f, SpectralBlob::Zero(), SpectralBlob::Zero() };

using LightPathWalker  = Walker<true>; // Enable russian roulette
using CameraPathWalker = Walker<true>; // Enable russian roulette
class IntBiDiInstance : public IIntegratorInstance {
public:
	explicit IntBiDiInstance(const BiDiParameters& parameters)
		: mParameters(parameters)
	{
		mLightPathWalker.MaxRayDepthHard  = mParameters.MaxLightRayDepthHard;
		mLightPathWalker.MaxRayDepthSoft  = mParameters.MaxLightRayDepthSoft;
		mCameraPathWalker.MaxRayDepthHard = mParameters.MaxCameraRayDepthHard;
		mCameraPathWalker.MaxRayDepthSoft = mParameters.MaxCameraRayDepthSoft;
	}

	virtual ~IntBiDiInstance() = default;

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

			// Calculate hero mis pdf
			const float matPDF = (spt.Ray.Flags & RF_Monochrome) ? out.PDF_S[0] : (out.PDF_S.sum() / PR_SPECTRAL_BLOB_SIZE);
			const float msiL   = MIS(1, sample.value().PDF_S, 1, matPDF);
			return Contribution{ msiL, out.Weight / sample.value().PDF_S, sample.value().Weight };
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
				EmissionEvalInput inL;
				inL.Entity		   = entity;
				inL.ShadingContext = ShadingContext::fromIP(session.threadID(), spt);
				EmissionEvalOutput outL;
				ems->eval(inL, outL, session);

				if (PR_LIKELY(!outL.Weight.isZero())) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), outL.Weight, spt.Ray, path);
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
					// TODO: Handle connections!

					// Infinite Lights
					// 1 sample per inf-light
					for (auto light : session.tile()->context()->scene()->infiniteLights()) {
						LightPathToken token;
						const auto contrib = infiniteLight(session, ip, token, light.get(), material_hit);

						if (contrib.MIS <= PR_EPSILON)
							continue;

						path.addToken(token);
						path.addToken(LightPathToken::Background());
						session.pushSpectralFragment(SpectralBlob(contrib.MIS), weight * contrib.Importance, contrib.Radiance, ip.Ray, path);
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

			// TODO
			/*handleLightPath(session, path, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");*/
			handleCameraPath(session, path, spt, sg.entity(), session.getMaterial(spt.Surface.Geometry.MaterialID));
			PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
		}
	}

	void onTile(RenderTileSession& session) override
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

private:
	const BiDiParameters mParameters;

	LightPathWalker mLightPathWalker;
	CameraPathWalker mCameraPathWalker;
};

class IntBiDi : public IIntegrator {
public:
	explicit IntBiDi(const BiDiParameters& parameters)
		: mParameters(parameters)
	{
	}

	virtual ~IntBiDi() = default;

	inline std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext*, size_t) override
	{
		return std::make_shared<IntBiDiInstance>(mParameters);
	}

private:
	const BiDiParameters mParameters;
};

class IntBiDiFactory : public IIntegratorFactory {
public:
	explicit IntBiDiFactory(const ParameterGroup& params)
	{
		mParameters.MaxCameraRayDepthHard = (size_t)params.getUInt("max_ray_depth", mParameters.MaxCameraRayDepthHard);
		mParameters.MaxCameraRayDepthSoft = std::min(mParameters.MaxCameraRayDepthHard, (size_t)params.getUInt("soft_max_ray_depth", mParameters.MaxCameraRayDepthSoft));
		mParameters.MaxLightRayDepthHard  = (size_t)params.getUInt("max_light_ray_depth", mParameters.MaxLightRayDepthHard);
		mParameters.MaxLightRayDepthSoft  = std::min(mParameters.MaxLightRayDepthHard, (size_t)params.getUInt("soft_max_light_ray_depth", mParameters.MaxLightRayDepthSoft));
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		return std::make_shared<IntBiDi>(mParameters);
	}

private:
	BiDiParameters mParameters;
};

class IntBiDiPlugin : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntBiDiFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "bidi", "bdpt", "bidirectional", "bidirect" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntBiDiPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)