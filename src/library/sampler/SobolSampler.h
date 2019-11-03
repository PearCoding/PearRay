#pragma once

#include "Sampler.h"
#include "math/SIMD.h"

namespace PR {

class PR_LIB SobolSampler : public Sampler {
public:
	explicit SobolSampler(uint32 samples,
						  uint32 baseX = 13, uint32 baseY = 47);
	~SobolSampler();

	float generate1D(uint32 index) override;
	Vector2f generate2D(uint32 index) override;

private:
	uint32 mSamples;

	simd_vector<float> mBaseXSamples;
	simd_vector<float> mBaseYSamples;
};
} // namespace PR
