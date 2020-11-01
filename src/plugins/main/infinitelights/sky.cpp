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

#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
// std::cos(ea.Elevation) has to be replaced by std::sin(ea.Elevation)??
constexpr float GROUND_PENALTY = 0.001f; // Scale down ground albedo in distribution to make sure mostly the sky is sampled

template <bool ExtendToGround>
class SkyLight : public IInfiniteLight {
public:
	SkyLight(const std::shared_ptr<ServiceObserver>& so, 
			 uint32 id, const std::string& name, const SkyModel& model,
			 float scale, const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mDistribution()
		, mModel(model)
		, mScale(scale)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
		, mSceneRadius(0)
		, mServiceObserver(so)
	{
		buildDistribution();

		if(mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene){
				mSceneRadius = scene->boundingSphere().radius() * 1.05f /*Scale a little bit*/;
			});
	}

	virtual ~SkyLight() {
		if(mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		auto ea = ElevationAzimuth::fromDirection(mInvTransform * in.Ray.Direction);

		if constexpr (ExtendToGround) {
			out.Radiance = radiance(in.Ray.WavelengthNM, ea);
			out.PDF_S	 = mDistribution->continuousPdf(
				   Vector2f(ea.Azimuth / AZIMUTH_RANGE, ea.Elevation / (2 * ELEVATION_RANGE) + 0.5f));
			const float f	  = std::cos(ea.Elevation);
			const float denom = 2 * PR_PI * PR_PI * f;
			out.PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			if (ea.Elevation < 0) {
				out.Radiance = SpectralBlob::Zero();
				out.PDF_S	 = 0;
			} else {
				out.Radiance = radiance(in.Ray.WavelengthNM, ea);
				out.PDF_S	 = mDistribution->continuousPdf(
					   Vector2f(ea.Azimuth / AZIMUTH_RANGE, ea.Elevation / ELEVATION_RANGE));
				const float f	  = std::cos(ea.Elevation);
				const float denom = 2 * PR_PI * PR_PI * f;
				out.PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
			}
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		Vector2f uv = mDistribution->sampleContinuous(Vector2f(in.RND(0), in.RND(1)), out.PDF_S);
		ElevationAzimuth ea;

		if constexpr (ExtendToGround)
			ea = ElevationAzimuth{ 2 * ELEVATION_RANGE * (uv(1) - 0.5f), AZIMUTH_RANGE * uv(0) };
		else
			ea = ElevationAzimuth{ ELEVATION_RANGE * uv(1), AZIMUTH_RANGE * uv(0) };

		out.Outgoing	  = mTransform * ea.toDirection();
		const float f	  = std::cos(ea.Elevation);
		const float denom = 2 * PR_PI * PR_PI * f;
		out.PDF_S *= (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;

		out.Radiance = radiance(in.WavelengthNM, ea);
		if (in.Point) // If we call it outside an intersection point, make light position such that lP - iP = direction
			out.LightPosition = in.Point->P + mSceneRadius * out.Outgoing;
		else if (in.SamplePosition) {
			out.LightPosition = 2 * mSceneRadius * out.Outgoing;
			// Instead of sampling position, sample direction again
			constexpr float CosAtan05 = 0.894427190999915f; // cos(atan(0.5)) = 2/sqrt(5)
			const Vector3f local	  = Sampling::uniform_cone(in.RND(2), in.RND(3), CosAtan05);
			out.Outgoing			  = Tangent::align(out.Outgoing, local);
			out.PDF_S *= Sampling::uniform_cone_pdf(CosAtan05) / mSceneRadius;
		}
	}

	float power() const override { return radiance(SpectralBlob(550.0f) /*TODO*/,
												   ElevationAzimuth::fromDirection(Vector3f(0, 0, 1)))
									   .mean(); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <SkyLight>:" << std::endl
			   << "    Scale:        " << mScale << std::endl
			   << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl
			   << "    Extended:     " << (ExtendToGround ? "true" : "false") << std::endl;
		// TODO
		return stream.str();
	}

private:
	inline void buildDistribution()
	{
		if constexpr (ExtendToGround) {
			mDistribution = std::make_shared<Distribution2D>(mModel.azimuthCount(), 2 * mModel.elevationCount());
		} else {
			mDistribution = std::make_shared<Distribution2D>(mModel.azimuthCount(), mModel.elevationCount());
		}

		PR_LOG(L_INFO) << "Generating 2d environment (" << mDistribution->width() << "x" << mDistribution->height() << ") of " << name() << std::endl;

		const SpectralBlob WVLS = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test
		mDistribution->generate([&](size_t x, size_t y) {
			const float azimuth = AZIMUTH_RANGE * x / (float)mModel.azimuthCount();
			float elevation;
			if constexpr (ExtendToGround) {
				elevation = (2 * ELEVATION_RANGE) * (y / (float)(2 * mModel.elevationCount()) - 0.5f);
			} else {
				elevation = ELEVATION_RANGE * y / (float)mModel.elevationCount();
			}

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

			float radiance = mModel.radiance(index, ea) * (1 - t) + mModel.radiance(index + 1, ea) * t;
			blob[i]		   = mScale * radiance;
		}
		return blob;
	}

	std::shared_ptr<Distribution2D> mDistribution;
	const std::shared_ptr<FloatSpectralNode> mGround;
	const SkyModel mModel;
	const float mScale;
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;

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
		Eigen::Matrix3f trans  = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		const float scale	   = ctx.parameters().getNumber("scale", 1.0f);
		auto ground_albedo	   = ctx.lookupSpectralNode("albedo", 0.15f);
		ElevationAzimuth sunEA = computeSunEA(ctx.parameters());
		//PR_LOG(L_INFO) << "Sun: " << PR_RAD2DEG * sunEA.Elevation << "° " << PR_RAD2DEG * sunEA.Azimuth << "°" << std::endl;

		const std::shared_ptr<ServiceObserver> so = ctx.hasEnvironment() ? ctx.environment()->serviceObserver() : nullptr;

		if (params.getBool("extend", true))
			return std::make_shared<SkyLight<true>>(so, id, name, SkyModel(ground_albedo, sunEA, ctx.parameters()), scale, trans);
		else
			return std::make_shared<SkyLight<false>>(so, id, name, SkyModel(ground_albedo, sunEA, ctx.parameters()), scale, trans);
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