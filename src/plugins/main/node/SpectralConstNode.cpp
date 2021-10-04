#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/ConstNode.h"
#include "shader/EquidistantSpectrumNode.h"
#include "shader/INodePlugin.h"

namespace PR {

class SpectralConstPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const float start = ctx.parameters().getNumber("start", 0.0f);
		const float end	  = ctx.parameters().getNumber("end", 0.0f);

		if (start >= end) {
			PR_LOG(L_ERROR) << "Invalid start and end wavelengths for spectrum given" << std::endl;
			return nullptr;
		}

		if (ctx.parameters().positionalParameterCount() == 0) {
			PR_LOG(L_ERROR) << "No values for spectrum given" << std::endl;
			return nullptr;
		}
		
		EquidistantSpectrum spectrum(ctx.parameters().positionalParameterCount(), start, end);
		for (size_t i = 0; i < ctx.parameters().positionalParameterCount(); ++i) {
			spectrum.at(i) = std::max(0.0f, ctx.parameters().getNumber(i, 0.0f));
		}

		return std::make_shared<EquidistantSpectrumNode>(spectrum);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "spectrum" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Spectral Value Node", "A constant spectral value")
			.Identifiers(getNames())
			.Inputs()
			.Number("start", "Wavelength start", 0.0f)
			.Number("end", "Wavelength end", 0.0f)
			// TODO
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SpectralConstPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)