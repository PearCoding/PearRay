#include "HaltonQMCSampler.h"

namespace PR {
HaltonQMCSampler::HaltonQMCSampler(uint32 samples, bool adaptive,
								   uint32 baseX, uint32 baseY, uint32 baseZ)
	: Sampler()
	, mSamples(samples)
	, mBaseXSamples(nullptr)
	, mBaseYSamples(nullptr)
	, mBaseZSamples(nullptr)
	, mAdaptive(adaptive)
	, mBaseX(baseX)
	, mBaseY(baseY)
	, mBaseZ(baseZ)
{
	PR_ASSERT(samples > 0, "Given sample count has to be greater than 0");

	mBaseXSamples = new float[samples];
	mBaseYSamples = new float[samples];
	mBaseZSamples = new float[samples];

	for (uint32 i = 0; i < samples; ++i) {
		mBaseXSamples[i] = halton(i, mBaseX);
		mBaseYSamples[i] = halton(i, mBaseY);
		mBaseZSamples[i] = halton(i, mBaseZ);
	}
}

HaltonQMCSampler::~HaltonQMCSampler()
{
	delete[] mBaseXSamples;
	delete[] mBaseYSamples;
	delete[] mBaseZSamples;
}

float HaltonQMCSampler::generate1D(uint32 index)
{
	if (!mAdaptive || index < mSamples)
		return mBaseXSamples[index % mSamples];
	else // To allow adaptive methods with higher samples
		return halton(index, mBaseX);
}

Eigen::Vector2f HaltonQMCSampler::generate2D(uint32 index)
{
	if (!mAdaptive || index < mSamples)
		return Eigen::Vector2f(mBaseXSamples[index % mSamples], mBaseYSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Eigen::Vector2f(halton(index, mBaseX),
							   halton(index, mBaseY));
}

Eigen::Vector3f HaltonQMCSampler::generate3D(uint32 index)
{
	if (!mAdaptive || index < mSamples)
		return Eigen::Vector3f(mBaseXSamples[index % mSamples],
							   mBaseYSamples[index % mSamples],
							   mBaseZSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Eigen::Vector3f(halton(index, mBaseX),
							   halton(index, mBaseY),
							   halton(index, mBaseZ));
}

float HaltonQMCSampler::halton(uint32 index, uint32 base)
{
	float result = 0;
	float f		 = 1;
	for (uint32 i = index; i > 0;) {
		f = f / base;
		result += f * (i % base);
		i = (uint32)std::floor(i / (float)base);
	}

	return result;
}
}
