#include "UniformSampler.h"
#include "Projection.h"

namespace PR
{
	UniformSampler::UniformSampler(Random& random, uint32 samples) :
		Sampler(), mRandom(random), mSamples(samples),
		mIndex(0)
	{
		m2D_X = std::sqrt(samples);
		m2D_Y = (mSamples + m2D_X - 1) / m2D_X;

		m3D_X = std::pow(samples, 1.0f/3);
		m3D_Y = m3D_X;
		m3D_Z = (mSamples + (m3D_X * m3D_Y) - 1) / (m3D_X * m3D_Y);
	}

	UniformSampler::~UniformSampler()
	{
	}

	float UniformSampler::generate1D()
	{
		float r = (mIndex % mSamples + 0.5f) / mSamples;
		mIndex++;
		return r;
	}

	PM::vec2 UniformSampler::generate2D()
	{
		auto r = PM::pm_Set((mIndex % m2D_X + 0.5f) / m2D_X,
			(mIndex / m2D_X + 0.5f) / m2D_Y);
		mIndex++;
		return r;
	}

	// TODO: Fix this! Not finished.
	PM::vec3 UniformSampler::generate3D()
	{
		auto r = PM::pm_Set((mIndex % (m3D_X*m3D_Y) + 0.5f) / (m3D_X*m3D_Y),
			(mIndex % (m3D_Y*m3D_Z) + 0.5f) /m3D_Y,
			(mIndex / (m3D_X*m3D_Y) + 0.5f) / m3D_Z);
		mIndex++;
		return r;
	}

	void UniformSampler::reset()
	{
		mIndex = 0;
	}
}