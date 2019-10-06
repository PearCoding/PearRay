#include "HaltonQMCSampler.h"

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

static vfloat haltonv(uint32 index, uint32 base)
{
	PR_SIMD_ALIGN float r[PR_SIMD_BANDWIDTH];
	for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i)
		r[i] = halton(index * PR_SIMD_BANDWIDTH + i, base);

	return simdpp::load(r);
}

HaltonQMCSampler::HaltonQMCSampler(uint32 samples,
								   uint32 baseX, uint32 baseY, uint32 baseZ)
	: Sampler()
	, mSamples(samples)
	, mBaseXSamples(samples)
	, mBaseYSamples(samples)
	, mBaseZSamples(samples)
	, mBaseX(baseX)
	, mBaseY(baseY)
	, mBaseZ(baseZ)
{
	PR_ASSERT(samples > 0, "Given sample count has to be greater than 0");

	for (uint32 i = 0; i < samples; ++i) {
		mBaseXSamples[i] = halton(i, mBaseX);
		mBaseYSamples[i] = halton(i, mBaseY);
		mBaseZSamples[i] = halton(i, mBaseZ);
	}
}

HaltonQMCSampler::~HaltonQMCSampler()
{
}

float HaltonQMCSampler::generate1D(uint32 index)
{
	if (index < mSamples)
		return mBaseXSamples[index % mSamples];
	else // To allow adaptive methods with higher samples
		return halton(index, mBaseX);
}

Vector2f HaltonQMCSampler::generate2D(uint32 index)
{
	if (index < mSamples)
		return Vector2f(mBaseXSamples[index % mSamples], mBaseYSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Vector2f(halton(index, mBaseX),
							   halton(index, mBaseY));
}

Vector3f HaltonQMCSampler::generate3D(uint32 index)
{
	if (index < mSamples)
		return Vector3f(mBaseXSamples[index % mSamples],
							   mBaseYSamples[index % mSamples],
							   mBaseZSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Vector3f(halton(index, mBaseX),
							   halton(index, mBaseY),
							   halton(index, mBaseZ));
}

void HaltonQMCSampler::generate1Dv(uint32 index, vfloat& s1)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples)
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
	else // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
}

void HaltonQMCSampler::generate2Dv(uint32 index, vfloat& s1, vfloat& s2)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples) {
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
		s2 = simdpp::load(&mBaseYSamples[index * PR_SIMD_BANDWIDTH]);
	} else { // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
		s2 = haltonv(index, mBaseY);
	}
}

void HaltonQMCSampler::generate3Dv(uint32 index, vfloat& s1, vfloat& s2, vfloat& s3)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples) {
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
		s2 = simdpp::load(&mBaseYSamples[index * PR_SIMD_BANDWIDTH]);
		s3 = simdpp::load(&mBaseZSamples[index * PR_SIMD_BANDWIDTH]);
	} else { // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
		s2 = haltonv(index, mBaseY);
		s3 = haltonv(index, mBaseZ);
	}
}
} // namespace PR
