#pragma once

#include "Sampler.h"

namespace PR {
#define PR_MJS_USE_RANDOM

/*
	  Correlated Multi-Jittered Sampling
	  by Andrew Kensler
	*/
class PR_LIB MultiJitteredSampler : public Sampler {
public:
	MultiJitteredSampler(Random& random, uint32 samples);
	~MultiJitteredSampler() = default;

	inline float generate1D(uint32 index) override;
	inline Eigen::Vector2f generate2D(uint32 index) override;
	inline Eigen::Vector3f generate3D(uint32 index) override;

private:
	inline static uint32 permute(uint32 i, uint32 l, uint32 p);
	inline static float randfloat(uint32 i, uint32 p);

	Random& mRandom;
	uint32 mSamples;

	uint32 m2D_X;
	uint32 m2D_Y;

	uint32 m3D_X;
	uint32 m3D_Y;
	uint32 m3D_Z;
};
}

#include "MultiJitteredSampler.inl"
