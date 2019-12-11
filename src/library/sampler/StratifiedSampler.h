#pragma once

#include "ISampler.h"

namespace PR {
class PR_LIB StratifiedSampler : public ISampler {
public:
	StratifiedSampler(Random& random, uint32 samples);
	~StratifiedSampler();

	float generate1D(uint32 index) override;
	Vector2f generate2D(uint32 index) override;

private:
	Random& mRandom;
	uint32 mSamples;

	uint32 m2D_X;
};
}
