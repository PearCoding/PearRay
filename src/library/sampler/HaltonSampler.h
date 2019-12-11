#pragma once

#include "ISampler.h"
#include "math/SIMD.h"

namespace PR {
/*
	  Quasi-MonteCarlo Sampling based on the Halton Sequence
	  https://en.wikipedia.org/wiki/Halton_sequence
	*/
class PR_LIB HaltonSampler : public ISampler {
public:
	explicit HaltonSampler(uint32 samples,
						   uint32 baseX = 13, uint32 baseY = 47);
	~HaltonSampler();

	float generate1D(uint32 index) override;
	Vector2f generate2D(uint32 index) override;

private:
	uint32 mSamples;

	simd_vector<float> mBaseXSamples;
	simd_vector<float> mBaseYSamples;

	const uint32 mBaseX;
	const uint32 mBaseY;
};
} // namespace PR
