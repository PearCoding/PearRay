#include "Environment.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "emission/IEmissionFactory.h"
#include "entity/IEntity.h"
#include "math/Projection.h"
#include "registry/Registry.h"

namespace PR {
class DiffuseEmission : public IEmission {
public:
	DiffuseEmission(uint32 id, const std::shared_ptr<FloatSpectralShadingSocket>& spec)
		: IEmission(id)
		, mRadiance(spec)
	{
	}

	// Given in luminous flux (cd/m^2)
	void eval(const LightEvalInput& in, LightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Weight = mRadiance->eval(in.Point);
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mRadiance;
};

class DiffuseEmissionFactory : public IEmissionFactory {
public:
	std::shared_ptr<IEmission> create(uint32 id, uint32 uuid, const SceneLoadContext& ctx) override
	{
		const Registry& reg = ctx.Env->registry();

		const std::string radianceName = reg.getForObject<std::string>(RG_EMISSION, uuid,
																	   "radiance", "");

		return std::make_shared<DiffuseEmission>(id, ctx.Env->getSpectralShadingSocket(radianceName, 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "diffuse", "standard", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DiffuseEmissionFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)