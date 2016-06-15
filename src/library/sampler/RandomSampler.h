#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB RandomSampler : public Sampler
	{
	public:
		RandomSampler(Random& random);
		~RandomSampler();

		void reset() override;

		float generate1D() override;
		PM::vec2 generate2D() override;
		PM::vec3 generate3D() override;

	private:
		Random& mRandom;
	};
}