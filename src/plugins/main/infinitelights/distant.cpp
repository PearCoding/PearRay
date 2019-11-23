#include "Environment.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"

namespace PR {
class DistantLight : public IInfiniteLight {
public:
	DistantLight(uint32 id, const std::string& name,
				 const Vector3f& direction,
				 const std::shared_ptr<FloatSpectralShadingSocket>& spec)
		: IInfiniteLight(id, name)
		, mDirection(direction)
		, mRadiance(spec)
	{
	}

	bool hasDeltaDistribution() const override { return true; }

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Weight = mRadiance->eval(in.Point);
		out.PDF_S  = std::numeric_limits<float>::infinity();
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		out.Weight   = mRadiance->eval(in.Point);
		out.Outgoing = mDirection_Cache;
		out.PDF_S	= std::numeric_limits<float>::infinity();
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <DistantLight>:" << std::endl
			   << "    Radiance: " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl
			   << "    LocalDirection: " << mDirection << std::endl
			   << "    GlobalDirection: " << mDirection_Cache << std::endl;

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
	std::shared_ptr<FloatSpectralShadingSocket> mRadiance;
	Vector3f mDirection_Cache;
};

class DistantLightFactory : public IInfiniteLightFactory {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, uint32 uuid, const Environment& env) override
	{
		const Registry& reg = env.registry();

		const std::string name		   = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
															   "name", "__unknown");
		const std::string radianceName = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																	   "radiance", "");
		const Vector3f direction	   = reg.getForObject<Vector3f>(RG_INFINITELIGHT, uuid,
																"direction", Vector3f(0, 0, 1));

		return std::make_shared<DistantLight>(id, name, direction, env.getSpectralShadingSocket(radianceName, 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sun", "distant", "direction" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DistantLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)