#include "Environment.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "emission/IEmissionPlugin.h"
#include "entity/IEntity.h"
#include "math/Projection.h"

namespace PR {
class DiffuseEmission : public IEmission {
public:
	DiffuseEmission(uint32 id, const std::shared_ptr<FloatSpectralNode>& spec)
		: IEmission(id)
		, mRadiance(spec)
	{
	}

	// Given in luminous flux (cd/m^2)
	void eval(const LightEvalInput& in, LightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Weight = mRadiance->eval(in.ShadingContext);
	}

private:
	std::shared_ptr<FloatSpectralNode> mRadiance;
};

class DiffuseEmissionPlugin : public IEmissionPlugin {
public:
	std::shared_ptr<IEmission> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<DiffuseEmission>(id,
												 ctx.lookupSpectralNode("radiance", 1));
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

PR_PLUGIN_INIT(PR::DiffuseEmissionPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)