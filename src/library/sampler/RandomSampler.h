#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB RandomSampler : public Sampler
	{
	public:
		RandomSampler(Random& random);
		~RandomSampler();
		
		float generate1D(uint32 index) override;
		PM::vec2 generate2D(uint32 index) override;
		PM::vec3 generate3D(uint32 index) override;

	private:
		Random& mRandom;
	};
}