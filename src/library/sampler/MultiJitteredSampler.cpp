#include "MultiJitteredSampler.h"

namespace PR {
MultiJitteredSampler::MultiJitteredSampler(Random& random, uint32 samples)
	: mRandom(random)
	, mSamples(samples)
{
	m2D_X = std::sqrt(samples);
	m2D_Y = (mSamples + m2D_X - 1) / m2D_X;

	m3D_X = std::cbrt(samples); //cube root
	m3D_Y = m3D_X;
	m3D_Z = (mSamples + (m3D_X * m3D_Y) - 1) / (m3D_X * m3D_Y);
}
} // namespace PR
