#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"

#include <vector>

namespace PR {
constexpr uint32 DEF_SAMPLE_COUNT = 128;
constexpr uint32 DEF_BASE_X		  = 13;
constexpr uint32 DEF_BASE_Y		  = 47;

static inline float halton(uint32 index, uint32 base)
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
				  uint32 baseX, uint32 baseY, uint32 burnin)
		: ISampler(samples)
		, mBaseXSamples(samples)
		, mBaseYSamples(samples)
		, mBaseX(baseX)
		, mBaseY(baseY)
		, mBurnin(burnin)
	{
		for (uint32 i = 0; i < samples; ++i) {
			mBaseXSamples[i] = halton(i + burnin, mBaseX);
			mBaseYSamples[i] = halton(i + burnin, mBaseY);
		}
	}

	virtual ~HaltonSampler() = default;

	float generate1D(Random&, uint32 index) override
	{
		if (index < maxSamples())
			return mBaseXSamples[index];
		else // To allow adaptive methods with higher samples
			return halton(index + mBurnin, mBaseX);
	}

	Vector2f generate2D(Random&, uint32 index) override
	{
		if (index < maxSamples())
			return Vector2f(mBaseXSamples[index], mBaseYSamples[index]);
		else // To allow adaptive methods with higher samples
			return Vector2f(halton(index + mBurnin, mBaseX),
							halton(index + mBurnin, mBaseY));
	}

private:
	std::vector<float> mBaseXSamples;
	std::vector<float> mBaseYSamples;

	const uint32 mBaseX;
	const uint32 mBaseY;
	const uint32 mBurnin;
};

// Same as halton but one dimension (in our case dim 2) being uniform.
// This sequence is not progressive! It falls back to being a halton sequence after the promised sample count is exceeded.
constexpr uint32 HAMMERSLEY_EVASIVE_BASE_Y = 47;
class HammersleySampler : public ISampler {
public:
	HammersleySampler(uint32 samples,
					  uint32 baseX, uint32 burnin)
		: ISampler(samples)
		, mBaseXSamples(samples)
		, mBaseYSamples(samples)
		, mBaseX(baseX)
		, mBurnin(burnin)
	{
		for (uint32 i = 0; i < samples; ++i) {
			mBaseXSamples[i] = halton(i + burnin, mBaseX);
			mBaseYSamples[i] = (0.5f + i) / samples;
		}
	}

	virtual ~HammersleySampler() = default;

	float generate1D(Random&, uint32 index) override
	{
		if (index < maxSamples())
			return mBaseXSamples[index];
		else // To allow adaptive methods with higher samples
			return halton(index + mBurnin, mBaseX);
	}

	Vector2f generate2D(Random&, uint32 index) override
	{
		if (index < maxSamples())
			return Vector2f(mBaseXSamples[index], mBaseYSamples[index]);
		else // To allow adaptive methods with higher samples (which is invalid for the hammersley sequence)
			return Vector2f(halton(index + mBurnin, mBaseX),
							halton(index + mBurnin, HAMMERSLEY_EVASIVE_BASE_Y));
	}

private:
	std::vector<float> mBaseXSamples;
	std::vector<float> mBaseYSamples;

	const uint32 mBaseX;
	const uint32 mBurnin;
};

class HaltonSamplerFactory : public ISamplerFactory {
public:
	explicit HaltonSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", DEF_SAMPLE_COUNT);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random&) const override
	{
		uint32 baseX  = (uint32)mParams.getUInt("base_x", DEF_BASE_X);
		uint32 baseY  = (uint32)mParams.getUInt("base_y", DEF_BASE_Y);
		uint32 burnin = (uint32)mParams.getUInt("burnin", std::max(baseX, baseY));
		return std::make_shared<HaltonSampler>(sample_count, baseX, baseY, burnin);
	}

private:
	ParameterGroup mParams;
};

class HammersleySamplerFactory : public ISamplerFactory {
public:
	explicit HammersleySamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", DEF_SAMPLE_COUNT);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random&) const override
	{
		uint32 baseX  = (uint32)mParams.getUInt("base_x", DEF_BASE_X);
		uint32 burnin = (uint32)mParams.getUInt("burnin", baseX);
		return std::make_shared<HammersleySampler>(sample_count, baseX, burnin);
	}

private:
	ParameterGroup mParams;
};

class HaltonSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(const std::string& name, const SceneLoadContext& ctx) override
	{
		// Skip sobol if progressive
		if (ctx.environment()->renderSettings().progressive) {
			PR_LOG(L_WARNING) << "Halton and hammersley sampler do not support progressive rendering. Using 'mjitt' instead" << std::endl;
			return ctx.loadSamplerFactory("mjitt", ctx.parameters());
		} else {
			if (name == "halton")
				return std::make_shared<HaltonSamplerFactory>(ctx.parameters());
			else
				return std::make_shared<HammersleySamplerFactory>(ctx.parameters());
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "halton", "hammersley" });
		return names;
	}

	PluginSpecification specification(const std::string& type_name) const override
	{
		if (type_name == "halton")
			return PluginSpecificationBuilder("Halton Sampler", "A quasi-random sampler based on the halton sequence in two dimensions with is not suited for progressive rendering")
				.Identifiers(getNames())
				.Inputs()
				.UInt("sample_count", "Sample count requested", DEF_SAMPLE_COUNT)
				.UInt("base_x", "Basis for x dimension", DEF_BASE_X)
				.UInt("base_y", "Basis for y dimension", DEF_BASE_Y)
				.UInt("burnin", "Burnin shift", std::max(DEF_BASE_X, DEF_BASE_Y))
				.Specification()
				.get();
		else
			return PluginSpecificationBuilder("Hammersley Sampler", "A quasi-random sampler based on the halton sequence in one dimension with is not suited for progressive rendering")
				.Identifiers(getNames())
				.Inputs()
				.UInt("sample_count", "Sample count requested", DEF_SAMPLE_COUNT)
				.UInt("base_x", "Basis for x dimension", DEF_BASE_X)
				.UInt("burnin", "Burnin shift", DEF_BASE_X)
				.Specification()
				.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::HaltonSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)