#include "Environment.h"
#include "PrettyPrint.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Concentric.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "renderer/RenderTileSession.h"
#include "scene/Scene.h"
#include "shader/NodeUtils.h"
#include "shader/ShadingContext.h"

namespace PR {
inline float calculatePosDiskRadius(float scene_radius, float cosTheta)
{
	return scene_radius * std::sqrt((1 - cosTheta) * (1 + cosTheta));
}
class DistantLight : public IInfiniteLight {
public:
	DistantLight(const std::shared_ptr<ServiceObserver>& so, 
				 uint32 id, const std::string& name, const Transformf& transform,
				 const Vector3f& direction,
				 const std::shared_ptr<FloatSpectralNode>& spec)
		: IInfiniteLight(id, name, transform)
		, mDirection(direction)
		, mIrradiance(spec)
		, mOutgoing_Cache((normalMatrix() * mDirection).normalized())
		, mSceneRadius(0)
		, mPosDiskRadius(0)
		, mServiceObserver(so)
	{
		Tangent::frame(mOutgoing_Cache, mDx, mDy);

		if(mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene){
				mSceneRadius   = scene->boundingSphere().radius();
				mPosDiskRadius = calculatePosDiskRadius(mSceneRadius, std::abs(mOutgoing_Cache(2)));
			});
	}

	virtual ~DistantLight() {
		if(mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	bool hasDeltaDistribution() const override { return true; }

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_ASSERT(false, "eval() for delta lights should never be called!");

		ShadingContext ctx;
		ctx.WavelengthNM = in.Ray.WavelengthNM;

		out.Radiance = mIrradiance->eval(ctx);
		out.PDF_S	 = PR_INF;
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		ShadingContext ctx;
		ctx.WavelengthNM = in.WavelengthNM;

		if (in.Point) {
			out.LightPosition = in.Point->P + mSceneRadius * mOutgoing_Cache;
		} else if (in.SamplePosition) {
			const Vector2f uv = mPosDiskRadius * Concentric::square2disc(Vector2f(in.RND(0), in.RND(1)));
			out.LightPosition = mSceneRadius * mOutgoing_Cache + uv(0) * mDx + uv(1) * mDy;
			// TODO: PDF?
		}

		out.Radiance = mIrradiance->eval(ctx); // As there is only one direction (delta), irradiance is equal to radiance
		out.Outgoing = mOutgoing_Cache;
		out.PDF_S	 = PR_INF;
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return NodeUtils::average(wvl, mIrradiance.get()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <DistantLight>:" << std::endl
			   << "    Irradiance: " << mIrradiance->dumpInformation() << std::endl
			   << "    Direction:  " << PR_FMT_MAT(mDirection) << std::endl;

		return stream.str();
	}

private:
	const Vector3f mDirection;
	const std::shared_ptr<FloatSpectralNode> mIrradiance;
	const Vector3f mOutgoing_Cache;

	Vector3f mDx;
	Vector3f mDy;

	float mSceneRadius;
	float mPosDiskRadius;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class DistantLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name	 = params.getString("name", "__unknown");
		const Vector3f direction = params.getVector3f("direction", Vector3f(0, 0, 1));

		return std::make_shared<DistantLight>(ctx.hasEnvironment() ? ctx.environment()->serviceObserver() : nullptr,
											  id, name, ctx.transform(), direction,
											  ctx.lookupSpectralNode("irradiance", 1));
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