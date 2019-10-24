#include "Environment.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"
#include "shader/ConstShadingSocket.h"

namespace PR {
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(uint32 id, const std::string& name,
					 const std::shared_ptr<FloatSpectralShadingSocket>& spec)
		: IInfiniteLight(id, name)
		, mRadiance(spec)
	{
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession& session) const override
	{
		out.Weight = mRadiance->eval(in.Point);
		float pdf;
		out.Outgoing = Projection::hemi(in.RND[0], in.RND[1], pdf);
		Tangent::align(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

		out.PDF_S = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance: " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		VirtualEntity::onFreeze(context);
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mRadiance;
};

class EnvironmentLightFactory : public IInfiniteLightFactory {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, uint32 uuid, const Environment& env) override
	{
		const Registry& reg = env.registry();

		const std::string name		   = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
															   "name", "__unknown");
		const std::string radianceName = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																	   "radiance", "");

		return std::make_shared<EnvironmentLight>(id, name, env.getSpectralShadingSocket(radianceName, 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "env", "environment", "background" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::EnvironmentLightFactory, "inf_environment", PR_PLUGIN_VERSION)