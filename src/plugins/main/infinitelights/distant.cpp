#include "Environment.h"
#include "PrettyPrint.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "shader/ShadingContext.h"

namespace PR {
class DistantLight : public IInfiniteLight {
public:
	DistantLight(uint32 id, const std::string& name,
				 const Vector3f& direction,
				 const std::shared_ptr<FloatSpectralNode>& spec)
		: IInfiniteLight(id, name)
		, mDirection(direction)
		, mIrradiance(spec)
	{
	}

	bool hasDeltaDistribution() const override { return true; }

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_ASSERT(false, "eval() for delta lights should never be called!");
		
		ShadingContext ctx;
		ctx.WavelengthNM = in.Ray.WavelengthNM;

		out.Weight = mIrradiance->eval(ctx);
		out.PDF_S  = PR_INF;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		ShadingContext ctx;
		ctx.WavelengthNM = in.Point.Ray.WavelengthNM;

		out.Weight	 = mIrradiance->eval(ctx);
		out.Outgoing = mDirection_Cache;
		out.PDF_S	 = PR_INF;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <DistantLight>:" << std::endl
			   << "    Irradiance: " << (mIrradiance ? mIrradiance->dumpInformation() : "NONE") << std::endl
			   << "    Direction:  " << PR_FMT_MAT(mDirection) << std::endl;

		return stream.str();
	}

	void beforeSceneBuild() override
	{
		IObject::beforeSceneBuild();

		mDirection_Cache = normalMatrix() * (-mDirection);
		mDirection_Cache.normalize();
	}

private:
	Vector3f mDirection;
	std::shared_ptr<FloatSpectralNode> mIrradiance;
	Vector3f mDirection_Cache;
};

class DistantLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name	 = params.getString("name", "__unknown");
		const Vector3f direction = params.getVector3f("direction", Vector3f(0, 0, 1));

		return std::make_shared<DistantLight>(id, name, direction,
											  ctx.Env->lookupSpectralNode(params.getParameter("irradiance"), 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "distant", "direction" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DistantLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)