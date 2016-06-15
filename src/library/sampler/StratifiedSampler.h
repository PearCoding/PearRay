#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB StratifiedSampler : public Sampler
	{
	public:
		StratifiedSampler(Random& random, uint32 samples);
		~StratifiedSampler();

		void reset() override;

		float generate1D() override;
		PM::vec2 generate2D() override;
		PM::vec3 generate3D() override;

	private:
		Random& mRandom;
		uint32 mSamples;

		uint32 mCurrent1DIndex;
		uint32 mCurrent2DIndex;
		uint32 mCurrent3DIndex;

		//uint8* m2DBitmask;
		//uint8* m3DBitmask;
	};
}