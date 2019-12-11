#include "StratifiedSampler.h"
#include "math/Projection.h"

namespace PR {
StratifiedSampler::StratifiedSampler(Random& random, uint32 samples)
	: ISampler()
	, mRandom(random)
	, mSamples(samples)
	, m2D_X(static_cast<uint32>(std::sqrt(samples)))
{
}

StratifiedSampler::~StratifiedSampler()
{
}

float StratifiedSampler::generate1D(uint32 index)
{
	auto ret = Projection::stratified(mRandom.getFloat(), index, mSamples);
	return ret;
}

// Need better strategy for 2D and 3D
Vector2f StratifiedSampler::generate2D(uint32 index)
{
	auto x = Projection::stratified(mRandom.getFloat(), index % m2D_X, mSamples);
	auto y = Projection::stratified(mRandom.getFloat(), index / m2D_X, mSamples);

	return Vector2f(x, y);
}
} // namespace PR
