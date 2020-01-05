#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
class UniformSampler : public ISampler {
public:
	UniformSampler(uint32 samples)
		: ISampler(samples)
		, m2D_X(static_cast<uint32>(std::sqrt(samples)))
		, m2D_Y((samples + m2D_X - 1) / m2D_X)
	{
	}

	virtual ~UniformSampler() = default;

	float generate1D(uint32 index) override
	{
		return (index % maxSamples() + 0.5f) / maxSamples();
	}

	Vector2f generate2D(uint32 index) override
	{
		return Vector2f((index % m2D_X + 0.5f) / m2D_X,
						(index / m2D_X + 0.5f) / m2D_Y);
	}

private:
	uint32 m2D_X;
	uint32 m2D_Y;
};

class UniformSamplerFactory : public ISamplerFactory {
public:
	UniformSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<ISampler> createInstance(Random&) const override
	{
		uint32 samples = (uint32)mParams.getUInt("sample_count", 128);
		return std::make_shared<UniformSampler>(samples);
	}

private:
	ParameterGroup mParams;
};

class UniformSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<UniformSamplerFactory>(ctx.Parameters);
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