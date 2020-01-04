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
	BlockFilterFactory(const ParameterGroup& params)
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
	std::shared_ptr<IFilterFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<BlockFilterFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "block", "blur" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::BlockFilterPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)