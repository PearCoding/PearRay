#include "StratifiedSampler.h"
#include "math/Projection.h"

namespace PR {
StratifiedSampler::StratifiedSampler(Random& random, uint32 samples)
	: Sampler()
	, mRandom(random)
	, mSamples(samples)
	, m2D_X(std::sqrt(samples))
	, m2D_Y((mSamples + m2D_X - 1) / m2D_X)
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
