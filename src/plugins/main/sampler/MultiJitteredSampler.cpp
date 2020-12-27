#include "Random.h"
#include "SceneLoadContext.h"
#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"

#include <vector>

namespace PR {
#define PR_MJS_USE_RANDOM
#define PR_MJS_CLIP

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
	MultiJitteredSampler(Random& random, uint32 samples, uint32 bins, uint32 seed)
		: ISampler(samples)
		, mRandom(random)
		, m1D(bins)
		, m2D_X(static_cast<uint32>(std::sqrt(bins)))
		, m2D_Y((bins + m2D_X - 1) / m2D_X)
		, mSeed(seed)
	{
	}

	float generate1D(uint32 index) override
	{
#ifndef PR_MJS_USE_RANDOM
		const float j = randfloat(index % m1D, mSeed * 0x711ad6a5);
#else
		const float j		= mRandom.getFloat();
#endif

		const float r = (index % m1D + j) / m1D;
		return r;
	}

	Vector2f generate2D(uint32 index) override
	{
#ifdef PR_MJS_CLIP
		constexpr uint32 FH = 0x51633e2d;
		constexpr uint32 F1 = 0x68bc21eb;
		constexpr uint32 F2 = 0x02e5be93;
		index				= permute(index, std::max(1u, maxSamples()), mSeed * FH);
#else
		constexpr uint32 F1 = 0xa511e9b3;
		constexpr uint32 F2 = 0x63d83595;
#endif

		const uint32 sx = permute(index % m2D_X, m2D_X, mSeed * F1);
		const uint32 sy = permute(index / m2D_X, m2D_Y, mSeed * F2);

#ifndef PR_MJS_USE_RANDOM
		const float jx = randfloat(index, mSeed * 0xa399d265);
		const float jy = randfloat(index, mSeed * 0x711ad6a5);
#else
		const float jx		= mRandom.getFloat();
		const float jy		= mRandom.getFloat();
#endif

#ifdef PR_MJS_CLIP
		const Vector2f r = Vector2f((sx + (sy + jx) / m2D_Y) / m2D_X,
									(index + jy) / std::max(1u, maxSamples()));
#else
		const Vector2f r	= Vector2f((index % m2D_X + (sy + jx) / m2D_Y) / m2D_X,
									   (index / m2D_X + (sx + jy) / m2D_X) / m2D_Y);
#endif

		return r;
	}

private:
	Random& mRandom;

	const uint32 m1D;
	const uint32 m2D_X;
	const uint32 m2D_Y;
	const uint32 mSeed;
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
		const uint32 PRIME = 14512081;
		return std::make_shared<MultiJitteredSampler>(rnd, sample_count,
													  mParams.getUInt("bins", std::max(1u, sample_count)),
													  mParams.getUInt("seed", PRIME ^ rnd.get32()));
	}

private:
	ParameterGroup mParams;
};

class MultiJitteredSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(const std::string&, const SceneLoadContext& ctx) override
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
