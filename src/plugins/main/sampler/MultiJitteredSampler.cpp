#include "Random.h"
#include "SceneLoadContext.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
#define PR_MJS_USE_RANDOM

/*
Reference:
	Correlated Multi-Jittered Sampling
	by Andrew Kensler
*/

static inline uint32 permute(uint32 i, uint32 l, uint32 p)
{
	uint32 w = l - 1;

	if (w == 0) {
		return 0;
	} else if ((l & w) == 0) // Power of 2
	{
		i ^= p;
		i *= 0xe170893d;
		i ^= p >> 16;
		i ^= (i & w) >> 4;
		i ^= p >> 8;
		i *= 0x0929eb3f;
		i ^= p >> 23;
		i ^= (i & w) >> 1;
		i *= 1 | p >> 27;
		i *= 0x6935fa69;
		i ^= (i & w) >> 11;
		i *= 0x74dcb303;
		i ^= (i & w) >> 2;
		i *= 0x9e501cc3;
		i ^= (i & w) >> 2;
		i *= 0xc860a3df;
		i &= w;
		i ^= i >> 5;
		return (i + p) & w;
	} else {
		w |= w >> 1;
		w |= w >> 2;
		w |= w >> 4;
		w |= w >> 8;
		w |= w >> 16;
		do {
			i ^= p;
			i *= 0xe170893d;
			i ^= p >> 16;
			i ^= (i & w) >> 4;
			i ^= p >> 8;
			i *= 0x0929eb3f;
			i ^= p >> 23;
			i ^= (i & w) >> 1;
			i *= 1 | p >> 27;
			i *= 0x6935fa69;
			i ^= (i & w) >> 11;
			i *= 0x74dcb303;
			i ^= (i & w) >> 2;
			i *= 0x9e501cc3;
			i ^= (i & w) >> 2;
			i *= 0xc860a3df;
			i &= w;
			i ^= i >> 5;
		} while (i >= l);
		return (i + p) % l;
	}
}

#ifndef PR_MJS_USE_RANDOM
static inline float randfloat(uint32 i, uint32 p)
{
	i ^= p;
	i ^= i >> 17;
	i ^= i >> 10;
	i *= 0xb36534e5;
	i ^= i >> 12;
	i ^= i >> 21;
	i *= 0x93fc4795;
	i ^= 0xdf6e307f;
	i ^= i >> 17;
	i *= 1 | p >> 18;
	return i * (1.0f / 4294967808.0f);
}
#endif //PR_MJS_USE_RANDOM

class MultiJitteredSampler : public ISampler {
public:
	MultiJitteredSampler(Random& random, uint32 samples)
		: ISampler(samples)
		, mRandom(random)
		, m2D_X(static_cast<uint32>(std::sqrt(samples)))
		, m2D_Y((samples + m2D_X - 1) / m2D_X)
	{
	}

	float generate1D(uint32 index) override
	{
		//uint32 s = permute(index % maxSamples(), maxSamples(), p * 0xa399d265);

#ifndef PR_MJS_USE_RANDOM
		uint32 p = mRandom.get32();
		float j  = randfloat(index % maxSamples(), p * 0x711ad6a5);
#else
		float j  = mRandom.getFloat();
#endif

		float r = (index % maxSamples() + j) / maxSamples();
		return r;
	}

	Vector2f generate2D(uint32 index) override
	{
		uint32 newIndex = index % maxSamples();

		uint32 p  = mRandom.get32();
		uint32 sx = permute(newIndex % m2D_X, m2D_X, p * 0xa511e9b3);
		uint32 sy = permute(newIndex / m2D_X, m2D_Y, p * 0x63d83595);

#ifndef PR_MJS_USE_RANDOM
		float jx = randfloat(newIndex, p * 0xa399d265);
		float jy = randfloat(newIndex, p * 0x711ad6a5);
#else
		float jx = mRandom.getFloat();
		float jy = mRandom.getFloat();
#endif

		auto r = Vector2f((newIndex % m2D_X + (sy + jx) / m2D_Y) / m2D_X,
						  (newIndex / m2D_X + (sx + jy) / m2D_X) / m2D_Y);
		return r;
	}

private:
	Random& mRandom;

	uint32 m2D_X;
	uint32 m2D_Y;
};

class MultiJitteredSamplerFactory : public ISamplerFactory {
public:
	explicit MultiJitteredSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", 128);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random& rnd) const override
	{
		return std::make_shared<MultiJitteredSampler>(rnd, sample_count);
	}

private:
	ParameterGroup mParams;
};

class MultiJitteredSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<MultiJitteredSamplerFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "multijittered", "multi_jittered", "jittered",
													  "multijitter", "multi_jitter", "jitter",
													  "mjitt", "jitt" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MultiJitteredSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
