#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Concentric.h"
#include "math/Projection.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "sampler/Distribution2D.h"
#include "scene/Scene.h"
#include "shader/ShadingContext.h"

#include "SampleUtils.h"

#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
// std::cos(ea.Elevation) has to be replaced by std::sin(ea.Elevation)??
constexpr float GROUND_PENALTY = 0.001f; // Scale down ground albedo in distribution to make sure mostly the sky is sampled

template <bool ExtendToGround>
class SkyLight : public IInfiniteLight {
public:
	SkyLight(const std::shared_ptr<ServiceObserver>& so,
			 uint32 id, const std::string& name, const Transformf& transform,
			 const SkyModel& model, bool allowCompensation)
		: IInfiniteLight(id, name, transform)
		, mDistribution()
		, mModel(model)
		, mSceneRadius(0)
		, mServiceObserver(so)
	{
		buildDistribution(allowCompensation);

		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius() * 1.05f /*Scale a little bit*/;
			});
	}

	virtual ~SkyLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		auto ea = ElevationAzimuth::fromDirection(invNormalMatrix() * in.Direction);

		if constexpr (ExtendToGround) {
			out.Radiance		= radiance(in.WavelengthNM, ea);
			out.Direction_PDF_S = mDistribution->continuousPdf(
				Vector2f(ea.Azimuth / AZIMUTH_RANGE, ea.Elevation / (2 * ELEVATION_RANGE) + 0.5f));
			const float f	  = std::cos(ea.Elevation);
			const float denom = 2 * PR_PI * PR_PI * f;
			out.Direction_PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			if (ea.Elevation < 0) {
				out.Radiance		= SpectralBlob::Zero();
				out.Direction_PDF_S = 0;
			} else {
				out.Radiance		= radiance(in.WavelengthNM, ea);
				out.Direction_PDF_S = mDistribution->continuousPdf(
					Vector2f(ea.Azimuth / AZIMUTH_RANGE, ea.Elevation / ELEVATION_RANGE));
				const float f	  = std::cos(ea.Elevation);
				const float denom = 2 * PR_PI * PR_PI * f;
				out.Direction_PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
			}
		}
	}

	void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out,
				   const RenderTileSession&) const override
	{
		Vector2f uv = mDistribution->sampleContinuous(Vector2f(in.DirectionRND(0), in.DirectionRND(1)), out.Direction_PDF_S);
		ElevationAzimuth ea;

		if constexpr (ExtendToGround)
			ea = ElevationAzimuth{ 2 * ELEVATION_RANGE * (uv(1) - 0.5f), AZIMUTH_RANGE * uv(0) };
		else
			ea = ElevationAzimuth{ ELEVATION_RANGE * uv(1), AZIMUTH_RANGE * uv(0) };

		out.Outgoing	  = normalMatrix() * ea.toDirection();
		const float f	  = std::cos(ea.Elevation);
		const float denom = 2 * PR_PI * PR_PI * f;
		out.Direction_PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;

		out.Radiance = radiance(in.WavelengthNM, ea);
	}

	void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out,
					  const RenderTileSession& session) const override
	{
		sampleDir(in, out, session);

		if (in.Point) { // If we call it outside an intersection point, make light position such that lP - iP = direction
			out.LightPosition  = in.Point->P + mSceneRadius * out.Outgoing;
			out.Position_PDF_A = 1;
		} else {
			out.LightPosition = sampleVisibleHemispherePos(in.PositionRND, out.Outgoing, mSceneRadius, out.Position_PDF_A);
		}
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return radiance(wvl, ElevationAzimuth::fromDirection(Vector3f(0, 0, 1))); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SkyLight>:" << std::endl
			   << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl
			   << "    Extended:     " << (ExtendToGround ? "true" : "false") << std::endl;
		return stream.str();
	}

private:
	inline void buildDistribution(bool allowCompensation)
	{
		if constexpr (ExtendToGround)
			mDistribution = std::make_shared<Distribution2D>(mModel.azimuthCount(), 2 * mModel.elevationCount());
		else
			mDistribution = std::make_shared<Distribution2D>(mModel.azimuthCount(), mModel.elevationCount());

		PR_LOG(L_INFO) << "Generating 2d environment (" << mDistribution->width() << "x" << mDistribution->height() << ") of " << name() << std::endl;

		const SpectralBlob WVLS = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test
		mDistribution->generate([&](size_t x, size_t y) {
			const float azimuth = AZIMUTH_RANGE * x / (float)mModel.azimuthCount();
			float elevation;
			if constexpr (ExtendToGround)
				elevation = (2 * ELEVATION_RANGE) * (y / (float)(2 * mModel.elevationCount()) - 0.5f);
			else
				elevation = ELEVATION_RANGE * y / (float)mModel.elevationCount();

			const float f	= std::cos(elevation);
			const float val = std::max(0.0f, f * radiance(WVLS, ElevationAzimuth{ elevation, azimuth }).maxCoeff());

			if constexpr (ExtendToGround) {
				if (elevation < 0.0f)
					return val * GROUND_PENALTY;
				else
					return val;
			} else {
				return val;
			}
		});

		if (allowCompensation)
			mDistribution->applyCompensation();
	}

	inline SpectralBlob radiance(const SpectralBlob& wvls, const ElevationAzimuth& ea) const
	{
		// If ea.Elevation is below zero it will be clamped to 0 in the model

		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float af	= std::max(0.0f, (wvls[i] - AR_SPECTRAL_START) / AR_SPECTRAL_DELTA);
			const int index = std::min<float>(AR_SPECTRAL_BANDS - 2, af);
			const float t	= std::min<float>(AR_SPECTRAL_BANDS - 1, af) - index;
			PR_ASSERT(t >= 0.0f && t <= 1.0f, "t must be between 0 and 1");

			blob[i] = mModel.radiance(index, ea) * (1 - t) + mModel.radiance(index + 1, ea) * t;
		}
		return blob;
	}

	std::shared_ptr<Distribution2D> mDistribution;
	const SkyModel mModel;

	float mSceneRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class SkyLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");

		auto ground_albedo			 = ctx.lookupSpectralNode("albedo", 0.15f);
		const bool allowCompensation = params.getBool("compensation", false); // Disabled per default, due to some bugs
		ElevationAzimuth sunEA		 = computeSunEA(ctx.parameters());
		//PR_LOG(L_INFO) << "Sun: " << PR_RAD2DEG * sunEA.Elevation << "° " << PR_RAD2DEG * sunEA.Azimuth << "°" << std::endl;

		const std::shared_ptr<ServiceObserver> so = ctx.hasEnvironment() ? ctx.environment()->serviceObserver() : nullptr;

		if (params.getBool("extend", true))
			return std::make_shared<SkyLight<true>>(so, id, name, ctx.transform(), SkyModel(ground_albedo, sunEA, ctx.parameters()), allowCompensation);
		else
			return std::make_shared<SkyLight<false>>(so, id, name, ctx.transform(), SkyModel(ground_albedo, sunEA, ctx.parameters()), allowCompensation);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sky" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SkyLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)