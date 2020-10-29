#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "shader/ShadingContext.h"

#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
// TODO: Add clear and intermediate sky which depend on the sun position
template <bool Cloudy>
class CIESimpleSkyLight : public IInfiniteLight {
public:
	CIESimpleSkyLight(uint32 id, const std::string& name,
					  const std::shared_ptr<FloatSpectralNode>& zenithTint,
					  const std::shared_ptr<FloatSpectralNode>& groundTint, const std::shared_ptr<FloatScalarNode>& groundBrightness,
					  const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mZenithTint(zenithTint)
		, mGroundTint(groundTint)
		, mGroundBrightness(groundBrightness)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Radiance = radiance(in.Ray.WavelengthNM, in.Ray.Direction);
		out.PDF_S	 = in.Point ? Sampling::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(in.Ray.Direction))) : 1.0f;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		out.Outgoing = Sampling::cos_hemi(in.RND[0], in.RND[1]);
		out.PDF_S	 = Sampling::cos_hemi_pdf(out.Outgoing(2));
		out.Radiance = radiance(in.WavelengthNM, out.Outgoing);
	}

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
		const Vector3f tD = mTransform * D;
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
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;
};

class CIESkyLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string& type, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name		= params.getString("name", "__unknown");
		const Eigen::Matrix3f trans = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());
		const auto zenithTint		= ctx.lookupSpectralNode("zenith", 1.0f);

		std::shared_ptr<FloatSpectralNode> groundTint;
		if (params.hasParameter("ground_tint"))
			groundTint = ctx.lookupSpectralNode("ground_tint", 0.0f);
		else
			groundTint = zenithTint;

		const auto groundBrightness = ctx.lookupScalarNode("ground_brightness", 0.2f);

		if (type == "uniform_sky")
			return std::make_shared<CIESimpleSkyLight<false>>(id, name, zenithTint, groundTint, groundBrightness, trans);
		else if (type == "cloudy_sky")
			return std::make_shared<CIESimpleSkyLight<true>>(id, name, zenithTint, groundTint, groundBrightness, trans);
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