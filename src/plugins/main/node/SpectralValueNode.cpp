#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/ConstNode.h"
#include "shader/INodePlugin.h"
#include "spectral/SpectralUpsampler.h"

namespace PR {

class SpectralValuePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string& type_name, const SceneLoadContext& ctx) override
	{
		const auto upsampler = ctx.environment()->defaultSpectralUpsampler();

		if (type_name == "refl" || type_name == "reflection") {
			ParametricBlob input = ParametricBlob(
				ctx.parameters().getParameter(0).getNumber(0.0f),
				ctx.parameters().getParameter(1).getNumber(0.0f),
				ctx.parameters().getParameter(2).getNumber(0.0f));

			const float max = input.maxCoeff();
			if (max > 1.0f)
				PR_LOG(L_WARNING) << "Given reflective rgb " << input << " contains coefficients above 1" << std::endl;

			ParametricBlob blob;
			upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
			return std::make_shared<ParametricSpectralNode>(blob);
		} else if (type_name == "illum" || type_name == "illumination") {
			ParametricBlob input = ParametricBlob(
				ctx.parameters().getParameter(0).getNumber(0.0f),
				ctx.parameters().getParameter(1).getNumber(0.0f),
				ctx.parameters().getParameter(2).getNumber(0.0f));

			ParametricBlob blob;
			const float max = input.maxCoeff();
			float power		= 1;
			if (max <= 0.0f) {
				upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
				power = 1;
			} else {
				const float scale		 = 2 * max;
				ParametricBlob scaled_in = input / scale;
				upsampler->prepare(&scaled_in(0), &scaled_in(1), &scaled_in(2), &blob(0), &blob(1), &blob(2), 1);
				power = scale;
			}
			return std::make_shared<ParametricScaledSpectralNode>(blob, power);
		} else {
			PR_ASSERT(false, "SpectralValueNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "refl", "reflection",
													  "illum", "illumination" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Spectral Value Node", "A constant spectral value")
			.Identifiers(getNames())
			.Inputs()
			.Number("value1", "First value", 0.0f)
			.Number("value2", "Second value", 0.0f)
			.Number("value3", "Third value", 0.0f)
			.Specification()
			.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::SpectralValuePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)