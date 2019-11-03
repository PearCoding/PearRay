#include "HaltonSampler.h"

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

HaltonSampler::HaltonSampler(uint32 samples,
							 uint32 baseX, uint32 baseY)
	: Sampler()
	, mSamples(samples)
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

HaltonSampler::~HaltonSampler()
{
}

float HaltonSampler::generate1D(uint32 index)
{
	if (index < mSamples)
		return mBaseXSamples[index % mSamples];
	else // To allow adaptive methods with higher samples
		return halton(index, mBaseX);
}

Vector2f HaltonSampler::generate2D(uint32 index)
{
	if (index < mSamples)
		return Vector2f(mBaseXSamples[index % mSamples], mBaseYSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Vector2f(halton(index, mBaseX),
						halton(index, mBaseY));
}
} // namespace PR
