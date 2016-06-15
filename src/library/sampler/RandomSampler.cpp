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

	float RandomSampler::generate1D(uint32 index)
	{
		return mRandom.getFloat();
	}

	PM::vec2 RandomSampler::generate2D(uint32 index)
	{
		return PM::pm_Set(mRandom.getFloat(), mRandom.getFloat());
	}

	PM::vec3 RandomSampler::generate3D(uint32 index)
	{
		return PM::pm_Set(mRandom.getFloat(), mRandom.getFloat(), mRandom.getFloat());
	}
}