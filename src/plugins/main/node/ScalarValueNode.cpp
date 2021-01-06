#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/ConstNode.h"
#include "shader/INodePlugin.h"

namespace PR {
// TODO: Make it variadic?
class ScalarValuePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return ctx.lookupScalarNode(ctx.parameters().getParameter(0));
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "constant", "const" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Scalar Value Node", "A scalar value")
			.Identifiers(getNames())
			.Inputs()
			.ScalarNode("value", "Value", 0.8f) // Why not number instead of scalarnode?
			.Specification()
			.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::ScalarValuePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)