#pragma once

#include "Sampler.h"
#include "math/SIMD.h"

namespace PR {
/*
	  Quasi-MonteCarlo Sampling based on the Halton Sequence
	  https://en.wikipedia.org/wiki/Halton_sequence
	*/
class PR_LIB HaltonQMCSampler : public Sampler {
public:
	explicit HaltonQMCSampler(uint32 samples,
							  uint32 baseX = 13, uint32 baseY = 47, uint32 baseZ = 89);
	~HaltonQMCSampler();

	float generate1D(uint32 index) const override;
	Eigen::Vector2f generate2D(uint32 index) const override;
	Eigen::Vector3f generate3D(uint32 index) const override;

	void generate1Dv(uint32 index, vfloat& s1) const override;
	void generate2Dv(uint32 index, vfloat& s1, vfloat& s1) const override;
	void generate3Dv(uint32 index, vfloat& s1, vfloat& s1, vfloat& s1) const override;

private:
	uint32 mSamples;

	simd_vector<float> mBaseXSamples;
	simd_vector<float> mBaseYSamples;
	simd_vector<float> mBaseZSamples;

	const uint32 mBaseX;
	const uint32 mBaseY;
	const uint32 mBaseZ;
};
} // namespace PR
