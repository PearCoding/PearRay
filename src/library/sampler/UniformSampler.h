#pragma once

#include "Sampler.h"

namespace PR {
class PR_LIB UniformSampler : public Sampler {
public:
	UniformSampler(uint32 samples);
	~UniformSampler();

	float generate1D(uint32 index) override;
	Vector2f generate2D(uint32 index) override;

private:
	uint32 mSamples;

	uint32 m2D_X;
	uint32 m2D_Y;
};
}
