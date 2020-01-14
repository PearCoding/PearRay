#include "Random.h"
#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"

namespace PR {
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

	std::shared_ptr<ISampler> createInstance(Random& rnd) const override
	{
		uint32 samples = (uint32)mParams.getUInt("sample_count", 128);
		return std::make_shared<RandomSampler>(rnd, samples);
	}

private:
	ParameterGroup mParams;
};

class RandomSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<RandomSamplerFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "random" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RandomSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
