#include "MultiJitteredSampler.h"

namespace PR {
MultiJitteredSampler::MultiJitteredSampler(Random& random, uint32 samples)
	: mRandom(random)
	, mSamples(samples)
	, m2D_X(static_cast<uint32>(std::sqrt(samples)))
	, m2D_Y((mSamples + m2D_X - 1) / m2D_X)
{
}
} // namespace PR
