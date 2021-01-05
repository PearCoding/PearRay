#include "Environment.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "emission/IEmissionPlugin.h"
#include "entity/IEntity.h"
#include "math/Projection.h"
#include "shader/NodeUtils.h"

namespace PR {
class DiffuseEmission : public IEmission {
public:
	DiffuseEmission(const std::shared_ptr<FloatSpectralNode>& spec)
		: IEmission()
		, mRadiance(spec)
	{
	}

	virtual ~DiffuseEmission() = default;

	// Given in radiance (W/(sr m^2))
	void eval(const EmissionEvalInput& in, EmissionEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Radiance = mRadiance->eval(in.ShadingContext);
	}

	SpectralBlob power(const SpectralBlob& wvl) const override { return NodeUtils::average(wvl, mRadiance.get()); }

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IEmission::dumpInformation()
			   << "  <DiffuseEmission>: " << mRadiance->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mRadiance;
};

class DiffuseEmissionPlugin : public IEmissionPlugin {
public:
	std::shared_ptr<IEmission> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<DiffuseEmission>(ctx.lookupSpectralNode("radiance", 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "diffuse", "standard", "default" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Diffuse Emission", "Standard emission for lights")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode("radiance", "Emitted energy in radiance", 1)
			.Specification()
			.get();
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DiffuseEmissionPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)