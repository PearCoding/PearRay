#pragma once

#include "Sampler.h"

namespace PR
{
	/*
	  Quasi-MonteCarlo Sampling based on the Halton Sequence
	  https://en.wikipedia.org/wiki/Halton_sequence
	*/
	class PR_LIB HaltonQMCSampler : public Sampler
	{
	public:
		HaltonQMCSampler(uint32 samples, bool adaptive=false,
			uint32 baseX = 13, uint32 baseY = 47, uint32 baseZ = 89);
		~HaltonQMCSampler();

		float generate1D(uint32 index) override;
		Eigen::Vector2f generate2D(uint32 index) override;
		Eigen::Vector3f generate3D(uint32 index) override;

	private:
		static float halton(uint32 index, uint32 base);

		uint32 mSamples;

		float* mBaseXSamples;
		float* mBaseYSamples;
		float* mBaseZSamples;

		const bool mAdaptive;
		const uint32 mBaseX;
		const uint32 mBaseY;
		const uint32 mBaseZ;
	};
}
