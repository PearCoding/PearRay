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
class DistantLight : public IInfiniteLight {
public:
	DistantLight(const std::shared_ptr<ServiceObserver>& so,
				 const std::string& name, const Transformf& transform,
				 const Vector3f& direction,
				 const std::shared_ptr<FloatSpectralNode>& spec)
		: IInfiniteLight(name, transform)
		, mDirection(direction)
		, mIrradiance(spec)
		, mOutgoing_Cache((normalMatrix() * mDirection).normalized())
		, mSceneRadius(0)
		, mServiceObserver(so)
		, mCBID(0)
	{
		Tangent::frame(mOutgoing_Cache, mDx, mDy);

		if (mServiceObserver)
			mCBID = mServiceObserver->registerAfterSceneBuild([this](Scene* scene) {
				mSceneRadius = scene->boundingSphere().radius();
			});
	}

	virtual ~DistantLight()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	bool hasDeltaDistribution() const override { return true; }

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_ASSERT(false, "eval() for delta lights should never be called!");

		ShadingContext ctx;
		ctx.WavelengthNM = in.WavelengthNM;

		out.Radiance		= mIrradiance->eval(ctx);
		out.Direction_PDF_S = 1;
	}

	void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out,
				   const RenderTileSession& session) const override
	{
		ShadingContext ctx;
		ctx.WavelengthNM	= in.WavelengthNM;
		ctx.ThreadIndex		= session.threadID();
		out.Radiance		= mIrradiance->eval(ctx); // As there is only one direction (delta), irradiance is equal to radiance
		out.Outgoing		= mOutgoing_Cache;
		out.Direction_PDF_S = 1;
	}

	void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out,
					  const RenderTileSession& session) const override
	{
		sampleDir(in, out, session);

		if (in.Point) {
			out.LightPosition  = in.Point->P + mSceneRadius * mOutgoing_Cache;
			out.Position_PDF_A = 1;
		} else {
			const Vector2f uv  = mSceneRadius * Concentric::square2disc(in.PositionRND);
			out.LightPosition  = mSceneRadius * mOutgoing_Cache + uv(0) * mDx + uv(1) * mDy;
			out.Position_PDF_A = 1 / (2 * PR_PI * mSceneRadius);
		}
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return NodeUtils::average(wvl, mIrradiance.get()); }
	SpectralRange spectralRange() const override { return mIrradiance->spectralRange(); }

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

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

class DistantLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name	 = params.getString("name", "__unknown");
		const Vector3f direction = params.getVector3f("direction", Vector3f(0, 0, 1));

		return std::make_shared<DistantLight>(ctx.environment()->serviceObserver(),
											  name, ctx.transform(), direction,
											  ctx.lookupSpectralNode("irradiance", 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "distant", "direction" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Distant Light", "An infinitly far away light from one direction")
			.Identifiers(getNames())
			.Inputs()
			.Vector("direction", "Direction the light is coming from", Vector3f(0, 0, 1))
			.SpectralNode("irradiance", "Irradiance", 1.0f)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DistantLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)