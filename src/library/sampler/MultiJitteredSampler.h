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

		float generate1D(uint32 index) override;
		PM::vec2 generate2D(uint32 index) override;
		PM::vec3 generate3D(uint32 index) override;

	private:
		static uint32 permute(uint32 i, uint32 l, uint32 p);
		static float randfloat(uint32 i, uint32 p);

		Random& mRandom;
		uint32 mSamples;

		uint32 mM;
		uint32 mN;
	};
}