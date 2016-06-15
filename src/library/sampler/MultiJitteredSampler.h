#pragma once

#include "Sampler.h"

namespace PR
{
	/*
	  Correlated Multi-Jittered Sampling
	  by Andrew Kensler
	*/
	class PR_LIB MultiJitteredSampler : public Sampler
	{
	public:
		MultiJitteredSampler(Random& random, uint32 samples);
		~MultiJitteredSampler();

		void reset() override;

		float generate1D() override;
		PM::vec2 generate2D() override;
		PM::vec3 generate3D() override;

	private:
		static uint32 permute(uint32 i, uint32 l, uint32 p);
		static float randfloat(uint32 i, uint32 p);

		Random& mRandom;
		uint32 mSamples;

		uint32 mM;
		uint32 mN;
		
		uint32 mIndex;
	};
}