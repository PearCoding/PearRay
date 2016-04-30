#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB Stratified3DSampler : public Sampler
	{
	public:
		Stratified3DSampler(uint32 x_samples, uint32 y_samples, uint32 z_samples);
		PM::vec generate(Random& rng);

	private:
		uint32 mXSamples;
		uint32 mYSamples;
		uint32 mZSamples;
		uint32 mXIndex;
		uint32 mYIndex;
		uint32 mZIndex;
	};
}