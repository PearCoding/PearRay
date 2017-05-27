#include "RandomSampler.h"

namespace PR {
RandomSampler::RandomSampler(Random& random)
	: Sampler()
	, mRandom(random)
{
}

RandomSampler::~RandomSampler()
{
}
}
