#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB RandomSampler : public Sampler
	{
	public:
		RandomSampler(Random& random);
		~RandomSampler();
		
		inline float generate1D(uint32 index) override
		{
			return mRandom.getFloat();
		}

		inline PM::vec2 generate2D(uint32 index) override
		{
			return mRandom.get2D();
		}

		inline PM::vec3 generate3D(uint32 index) override
		{
			return mRandom.get3D();
		}

	private:
		Random& mRandom;
	};
}