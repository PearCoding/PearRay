#pragma once

#include "Sampler.h"

namespace PR {

class PR_LIB SobolSampler : public Sampler {
public:
	explicit SobolSampler(Random& random, uint32 samples);
	~SobolSampler();

	float generate1D(uint32 index) override;
	Vector2f generate2D(uint32 index) override;

private:
	Random& mRandom;

	std::vector<float> mSamples1D;
	std::vector<Vector2f> mSamples2D;
};
} // namespace PR
