#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB Stratified2DSampler : public Sampler
	{
	public:
		Stratified2DSampler(uint32 x_samples, uint32 y_samples);
		PM::vec generate(Random& rng);

	private:
		uint32 mXSamples;
		uint32 mYSamples;
		uint32 mXIndex;
		uint32 mYIndex;
	};
}