#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
/// A very bad sampler for test purposes
class UniformSampler : public ISampler {
public:
	explicit UniformSampler(uint32 samples)
		: ISampler(samples)
	{
	}

	virtual ~UniformSampler() = default;

	float generate1D(uint32) override
	{
		return 0.5f;
	}

	Vector2f generate2D(uint32) override
	{
		return Vector2f(0.5f, 0.5f);
	}
};

class UniformSamplerFactory : public ISamplerFactory {
public:
	explicit UniformSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", 128);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random&) const override
	{
		return std::make_shared<UniformSampler>(sample_count);
	}

private:
	ParameterGroup mParams;
};

class UniformSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<UniformSamplerFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "uniform" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::UniformSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)