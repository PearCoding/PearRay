#pragma once

#include "Sampler.h"

namespace PR {
class PR_LIB RandomSampler : public Sampler {
public:
	explicit RandomSampler(Random& random);
	~RandomSampler();

	inline float generate1D(uint32 index) override { return mRandom.getFloat(); }
	inline Eigen::Vector2f generate2D(uint32 index) override { return mRandom.get2D(); }
	inline Eigen::Vector3f generate3D(uint32 index) override { return mRandom.get3D(); }

private:
	Random& mRandom;
};
}
