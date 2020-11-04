#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "scene/Scene.h"
#include "shader/ShadingContext.h"

#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
// TODO: Add clear and intermediate sky which depend on the sun position
template <bool Cloudy>
class CIESimpleSkyLight : public IInfiniteLight {
public:
	CIESimpleSkyLight(const std::shared_ptr<ServiceObserver>& so,
					  uint32 id, const std::string& name, const Transformf& transform,
					  const std::shared_ptr<FloatSpectralNode>& zenithTint,
					  const std::shared_ptr<FloatSpectralNode>& groundTint, const std::shared_ptr<FloatScalarNode>& groundBrightness)
		: IInfiniteLight(id, name, transform)
		, mZenithTint(zenithTint)
		, mGroundTint(groundTint)
		, mGroundBrightness(groundBrightness)
		, mSceneRadius(0)
		, mServiceObserver(so)
	{
		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius() * 1.05f /*Scale a little bit*/;
			});
	}

	virtual ~CIESimpleSkyLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Radiance = radiance(in.Ray.WavelengthNM, in.Ray.Direction);
		out.PDF_S	 = in.Point ? Sampling::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(in.Ray.Direction))) : 1.0f;
	}

	inline static Vector3f sampleDir(float u0, float u1, float& pdf)
	{
		Vector3f out = Sampling::cos_hemi(u0, u1);
		pdf			 = Sampling::cos_hemi_pdf(out(2));
		return out;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		out.Outgoing	  = normalMatrix() * sampleDir(in.RND[0], in.RND[1], out.PDF_S);
		out.LightPosition = mSceneRadius * out.Outgoing;
		out.Radiance	  = radiance(in.WavelengthNM, out.Outgoing);
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return radiance(wvl, Vector3f(0, 0, 1)); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IInfiniteLight::dumpInformation();

		if constexpr (Cloudy)
			stream << "  <CIEUniformSkyLight>:" << std::endl;
		else
			stream << "  <CIECloudySkyLight>:" << std::endl;

		stream << "    ZenithTint:       " << mZenithTint->dumpInformation() << std::endl
			   << "    GroundTint:       " << mGroundTint->dumpInformation() << std::endl
			   << "    GroundBrightness: " << mGroundBrightness->dumpInformation() << std::endl;
		return stream.str();
	}

private:
	inline SpectralBlob radiance(const SpectralBlob& wvl, const Vector3f& D) const
	{
		const Vector3f tD = invNormalMatrix() * D;
		ShadingContext ctx;
		ctx.UV			 = Spherical::uv_from_normal(tD);
		ctx.WavelengthNM = wvl;

		const float skybrightness = std::pow(tD(2) + 1.01f, 10);
		const float a			  = skybrightness;
		const float b			  = 1 / skybrightness;
		const float denom		  = 1 / (a + b);

		float c1 = 1;
		float c2 = 1;
		if constexpr (Cloudy) {
			c1 = (1 + 2.0f * tD(2)) / 3.0f;
			c2 = 0.7777777f;
		}
		return (mZenithTint->eval(ctx) * (c1 * a) + mGroundTint->eval(ctx) * (mGroundBrightness->eval(ctx) * c2 * b)) * denom;
	}

	const std::shared_ptr<FloatSpectralNode> mZenithTint;
	const std::shared_ptr<FloatSpectralNode> mGroundTint;
	const std::shared_ptr<FloatScalarNode> mGroundBrightness;

	float mSceneRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class CIESkyLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string& type, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unknown");
		const auto zenithTint  = ctx.lookupSpectralNode("zenith", 1.0f);

		std::shared_ptr<FloatSpectralNode> groundTint;
		if (params.hasParameter("ground_tint"))
			groundTint = ctx.lookupSpectralNode("ground_tint", 0.0f);
		else
			groundTint = zenithTint;

		const auto groundBrightness = ctx.lookupScalarNode("ground_brightness", 0.2f);

		const std::shared_ptr<ServiceObserver> so = ctx.hasEnvironment() ? ctx.environment()->serviceObserver() : nullptr;

		if (type == "uniform_sky")
			return std::make_shared<CIESimpleSkyLight<false>>(so, id, name, ctx.transform(), zenithTint, groundTint, groundBrightness);
		else if (type == "cloudy_sky")
			return std::make_shared<CIESimpleSkyLight<true>>(so, id, name, ctx.transform(), zenithTint, groundTint, groundBrightness);
		else {
			PR_ASSERT(false, "CIESkyFactory plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "uniform_sky", "cloudy_sky" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CIESkyLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)