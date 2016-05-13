#include "RandomSampler.h"

namespace PR
{
	RandomSampler::RandomSampler(Random& random) :
		Sampler(), mRandom(random)
	{
	}

	RandomSampler::~RandomSampler()
	{
	}

	float RandomSampler::generate1D()
	{
		return mRandom.getFloat();
	}

	PM::vec2 RandomSampler::generate2D()
	{
		return PM::pm_Set(mRandom.getFloat(), mRandom.getFloat());
	}

	PM::vec3 RandomSampler::generate3D()
	{
		return PM::pm_Set(mRandom.getFloat(), mRandom.getFloat(), mRandom.getFloat());
	}
}