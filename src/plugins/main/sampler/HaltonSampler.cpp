#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
static float halton(uint32 index, uint32 base)
{
	float result = 0;
	float f		 = 1;
	for (uint32 i = index; i > 0;) {
		f = f / base;
		result += f * (i % base);
		i = static_cast<uint32>(std::floor(i / static_cast<float>(base)));
	}

	return result;
}
/*
	Quasi-MonteCarlo Sampling based on the Halton Sequence
	https://en.wikipedia.org/wiki/Halton_sequence
*/
class HaltonSampler : public ISampler {
public:
	HaltonSampler(uint32 samples,
				  uint32 baseX, uint32 baseY)
		: ISampler(samples)
		, mBaseXSamples(samples)
		, mBaseYSamples(samples)
		, mBaseX(baseX)
		, mBaseY(baseY)
	{
		PR_ASSERT(samples > 0, "Given sample count has to be greater than 0");

		for (uint32 i = 0; i < samples; ++i) {
			mBaseXSamples[i] = halton(i, mBaseX);
			mBaseYSamples[i] = halton(i, mBaseY);
		}
	}

	virtual ~HaltonSampler() = default;

	float generate1D(uint32 index)
	{
		if (index < maxSamples())
			return mBaseXSamples[index % maxSamples()];
		else // To allow adaptive methods with higher samples
			return halton(index, mBaseX);
	}

	Vector2f generate2D(uint32 index)
	{
		if (index < maxSamples())
			return Vector2f(mBaseXSamples[index % maxSamples()], mBaseYSamples[index % maxSamples()]);
		else // To allow adaptive methods with higher samples
			return Vector2f(halton(index, mBaseX),
							halton(index, mBaseY));
	}

private:
	std::vector<float> mBaseXSamples;
	std::vector<float> mBaseYSamples;

	const uint32 mBaseX;
	const uint32 mBaseY;
};

class HaltonSamplerFactory : public ISamplerFactory {
public:
	explicit HaltonSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<ISampler> createInstance(Random&) const override
	{
		uint32 samples = (uint32)mParams.getUInt("sample_count", 128);
		uint32 baseX   = (uint32)mParams.getUInt("base_x", 13);
		uint32 baseY   = (uint32)mParams.getUInt("base_y", 47);
		return std::make_shared<HaltonSampler>(samples, baseX, baseY);
	}

private:
	ParameterGroup mParams;
};

class HaltonSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<HaltonSamplerFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "halton" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::HaltonSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)