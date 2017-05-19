#pragma once

#include "Sampler.h"

namespace PR
{
	class PR_LIB StratifiedSampler : public Sampler
	{
	public:
		StratifiedSampler(Random& random, uint32 samples);
		~StratifiedSampler();

		float generate1D(uint32 index) override;
		Eigen::Vector2f generate2D(uint32 index) override;
		Eigen::Vector3f generate3D(uint32 index) override;

	private:
		Random& mRandom;
		uint32 mSamples;

		uint32 m2D_X;
		uint32 m2D_Y;
	};
}
