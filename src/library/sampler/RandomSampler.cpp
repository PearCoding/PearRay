#include "RandomSampler.h"

namespace PR {
RandomSampler::RandomSampler(Random& random)
	: ISampler()
	, mRandom(random)
{
}

RandomSampler::~RandomSampler()
{
}
}
