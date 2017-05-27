#include "UniformSampler.h"
#include "math/Projection.h"

namespace PR {
UniformSampler::UniformSampler(Random& random, uint32 samples)
	: Sampler()
	, mRandom(random)
	, mSamples(samples)
{
	m2D_X = std::sqrt(samples);
	m2D_Y = (mSamples + m2D_X - 1) / m2D_X;

	m3D_X = std::cbrt(samples); //cube root
	m3D_Y = m3D_X;
	m3D_Z = (mSamples + (m3D_X * m3D_Y) - 1) / (m3D_X * m3D_Y);
}

UniformSampler::~UniformSampler()
{
}

float UniformSampler::generate1D(uint32 index)
{
	float r = (index % mSamples + 0.5f) / mSamples;
	return r;
}

Eigen::Vector2f UniformSampler::generate2D(uint32 index)
{
	auto r = Eigen::Vector2f((index % m2D_X + 0.5f) / m2D_X,
							 (index / m2D_X + 0.5f) / m2D_Y);
	return r;
}

// TODO: Fix this! Not finished.
Eigen::Vector3f UniformSampler::generate3D(uint32 index)
{
	auto r = Eigen::Vector3f((index % (m3D_X * m3D_Y) + 0.5f) / (m3D_X * m3D_Y),
							 (index % (m3D_Y * m3D_Z) + 0.5f) / m3D_Y,
							 (index / (m3D_X * m3D_Y) + 0.5f) / m3D_Z);
	return r;
}
}
