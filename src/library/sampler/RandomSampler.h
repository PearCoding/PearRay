#pragma once

#include "Sampler.h"

namespace PR {
class PR_LIB RandomSampler : public Sampler {
public:
	explicit RandomSampler(Random& random);
	~RandomSampler();

	inline float generate1D(uint32 index) override { return mRandom.getFloat(); }
	inline Vector2f generate2D(uint32 index) override { return mRandom.get2D(); }

private:
	Random& mRandom;
};
}
