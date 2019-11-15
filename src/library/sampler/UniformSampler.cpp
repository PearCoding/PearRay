#include "UniformSampler.h"
#include "math/Projection.h"

namespace PR {
UniformSampler::UniformSampler(uint32 samples)
	: Sampler()
	, mSamples(samples)
	, m2D_X(static_cast<uint32>(std::sqrt(samples)))
	, m2D_Y((mSamples + m2D_X - 1) / m2D_X)
{
}

UniformSampler::~UniformSampler()
{
}

float UniformSampler::generate1D(uint32 index)
{
	return (index % mSamples + 0.5f) / mSamples;
}

Vector2f UniformSampler::generate2D(uint32 index)
{
	return Vector2f((index % m2D_X + 0.5f) / m2D_X,
					(index / m2D_X + 0.5f) / m2D_Y);
}
} // namespace PR
