#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB Stratified1DSampler : public Sampler
	{
	public:
		Stratified1DSampler(uint32 samples);
		PM::vec generate(Random& rng);

	private:
		uint32 mSamples;
		uint32 mIndex;
	};
}