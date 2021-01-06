#include "SceneLoadContext.h"
#include "filter/IFilter.h"
#include "filter/IFilterFactory.h"
#include "filter/IFilterPlugin.h"

namespace PR {
class BlockFilter : public IFilter {
public:
	explicit BlockFilter(int radius = 1)
		: mRadius(radius)
	{
	}

	int radius() const override { return mRadius; }
	float evalWeight(float, float) const override { return 1.0f / ((2 * mRadius + 1) * (2 * mRadius + 1)); }

private:
	int mRadius;
};

class BlockFilterFactory : public IFilterFactory {
public:
	explicit BlockFilterFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IFilter> createInstance() const override
	{
		int radius = (int)mParams.getInt("radius", 3);
		return std::make_shared<BlockFilter>(radius);
	}

private:
	ParameterGroup mParams;
};

class BlockFilterPlugin : public IFilterPlugin {
public:
	std::shared_ptr<IFilterFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<BlockFilterFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "block", "blur" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Block Filter", "Simple filter based on the block funtion")
			.Identifiers(getNames())
			.Inputs()
			.UInt("radius", "Radius of filter", 3)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::BlockFilterPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)