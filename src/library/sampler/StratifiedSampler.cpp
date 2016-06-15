#include "StratifiedSampler.h"
#include "Projection.h"

namespace PR
{
	StratifiedSampler::StratifiedSampler(Random& random, uint32 samples) :
		Sampler(), mRandom(random), mSamples(samples),
		mCurrent1DIndex(0), mCurrent2DIndex(0), mCurrent3DIndex(0)
	{
		/*m2DBitmask = new uint8[samples];
		m3DBitmask = new uint8[samples * samples];*/
	}

	StratifiedSampler::~StratifiedSampler()
	{
		/*delete[] m2DBitmask;
		delete[] m3DBitmask;*/
	}

	float StratifiedSampler::generate1D()
	{
		auto ret = Projection::stratified(mRandom.getFloat(), mCurrent1DIndex, mSamples);

		mCurrent1DIndex++;
		if (mCurrent1DIndex >= mSamples)
			mCurrent1DIndex = 0;

		return ret;
	}

	// Need better strategy for 2D and 3D
	PM::vec2 StratifiedSampler::generate2D()
	{
		auto x = Projection::stratified(mRandom.getFloat(), mCurrent2DIndex, mSamples);
		auto y = Projection::stratified(mRandom.getFloat(), mRandom.get32(0, mSamples), mSamples);

		mCurrent2DIndex++;
		if (mCurrent2DIndex >= mSamples)
			mCurrent2DIndex = 0;

		return PM::pm_Set(x, y);
	}

	PM::vec3 StratifiedSampler::generate3D()
	{
		auto x = Projection::stratified(mRandom.getFloat(), mCurrent2DIndex, mSamples);
		auto y = Projection::stratified(mRandom.getFloat(), mRandom.get32(0, mSamples), mSamples);
		auto z = Projection::stratified(mRandom.getFloat(), mRandom.get32(0, mSamples), mSamples);

		mCurrent3DIndex++;
		if (mCurrent3DIndex >= mSamples)
			mCurrent3DIndex = 0;

		return PM::pm_Set(x, y, z);
	}

	void StratifiedSampler::reset()
	{
		mCurrent1DIndex = 0;
		mCurrent2DIndex = 0;
		mCurrent3DIndex = 0;
	}
}