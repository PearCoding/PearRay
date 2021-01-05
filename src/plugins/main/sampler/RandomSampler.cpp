#include "Random.h"
#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"

namespace PR {
constexpr uint32 DEF_SAMPLE_COUNT = 128;

class RandomSampler : public ISampler {
public:
	RandomSampler(Random& random, uint32 samples)
		: ISampler(samples)
		, mRandom(random)
	{
	}

	virtual ~RandomSampler() = default;

	inline float generate1D(uint32) override { return mRandom.getFloat(); }
	inline Vector2f generate2D(uint32) override { return mRandom.get2D(); }

private:
	Random& mRandom;
};

class RandomSamplerFactory : public ISamplerFactory {
public:
	explicit RandomSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", DEF_SAMPLE_COUNT);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random& rnd) const override
	{
		return std::make_shared<RandomSampler>(rnd, sample_count);
	}

private:
	ParameterGroup mParams;
};

class RandomSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<RandomSamplerFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "random" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Random Sampler", "A progressive uniform sampler with bad quality")
			.Identifiers(getNames())
			.Inputs()
			.UInt("sample_count", "Sample count requested", DEF_SAMPLE_COUNT)
			.Specification()
			.get();
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RandomSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
