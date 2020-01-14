#include "Random.h"
#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {

#include "SobolSamplerData.inl"

// Get index from the right of the first zero bit
static size_t irfz(uint32 n)
{
	size_t counter = 1;
	while (n & 1) {
		n >>= 1;
		++counter;
	}
	return counter;
}

class SobolSampler : public ISampler {
public:
	SobolSampler(Random& random, uint32 samples)
		: ISampler(samples)
		, mRandom(random)
	{
		PR_ASSERT(samples > 0, "Given sample count has to be greater than 0");

		mSamples1D.resize(samples);
		mSamples2D.resize(samples);

		mSamples1D[0]		  = 0.0f;
		mSamples2D[0]		  = Vector2f(0, 0);
		sobol_entry_t last[2] = { 0, 0 };
		for (uint32 i = 1; i < samples; ++i) {
			size_t cin			   = irfz(i - 1);
			sobol_entry_t entry[2] = {
				last[0] ^ sobol_V[0][cin - 1],
				last[1] ^ sobol_V[1][cin - 1]
			};

			last[0] = entry[0];
			last[1] = entry[1];

			mSamples1D[i] = static_cast<float>(entry[0] / static_cast<double>(std::numeric_limits<sobol_entry_t>::max()));
			mSamples2D[i] = Vector2f(
				mSamples1D.at(i),
				static_cast<float>(entry[1] / static_cast<double>(std::numeric_limits<sobol_entry_t>::max())));
		}

		// Shuffle the order of the samples
		std::shuffle(mSamples1D.begin(), mSamples1D.end(), mRandom);
		std::shuffle(mSamples2D.begin(), mSamples2D.end(), mRandom);
	}

	virtual ~SobolSampler() = default;

	float generate1D(uint32 index)
	{
		if (mSamples1D.size() <= index) {
			return mRandom.getFloat();
		} else {
			return mSamples1D.at(index);
		}
	}

	Vector2f generate2D(uint32 index)
	{
		if (mSamples2D.size() <= index) {
			return mRandom.get2D();
		} else {
			return mSamples2D.at(index);
		}
	}

private:
	Random& mRandom;

	std::vector<float> mSamples1D;
	std::vector<Vector2f> mSamples2D;
};

class SobolSamplerFactory : public ISamplerFactory {
public:
	explicit SobolSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<ISampler> createInstance(Random& rnd) const override
	{
		uint32 samples = (uint32)mParams.getUInt("sample_count", 128);
		return std::make_shared<SobolSampler>(rnd, samples);
	}

private:
	ParameterGroup mParams;
};

class SobolSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<SobolSamplerFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sobol" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SobolSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)