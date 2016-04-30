#include "Stratified3DSampler.h"
#include "Projection.h"

namespace PR
{
	Stratified3DSampler::Stratified3DSampler(uint32 x_samples, uint32 y_samples, uint32 z_samples) :
		Sampler(), mXSamples(x_samples), mYSamples(y_samples), mZSamples(z_samples), mXIndex(0), mYIndex(0), mZIndex(0)
	{

	}

	PM::vec Stratified3DSampler::generate(Random& rng)
	{
		auto ret = PM::pm_Set(Projection::stratified(rng.getFloat(), mXIndex, mXSamples),
			Projection::stratified(rng.getFloat(), mYIndex, mYSamples),
			Projection::stratified(rng.getFloat(), mZIndex, mZSamples));

		mXIndex++;
		if (mXIndex >= mXSamples)
		{
			mXIndex = 0;
			mYIndex++;
			if (mYIndex >= mYSamples)
			{
				mYIndex = 0;
				mZIndex++;
			}
		}
		return ret;
	}
}