#include "Stratified2DSampler.h"
#include "Projection.h"

namespace PR
{
	Stratified2DSampler::Stratified2DSampler(uint32 x_samples, uint32 y_samples) :
		Sampler(), mXSamples(x_samples), mYSamples(y_samples), mXIndex(0), mYIndex(0)
	{

	}

	PM::vec Stratified2DSampler::generate(Random& rng)
	{
		auto ret = PM::pm_Set(Projection::stratified(rng.getFloat(), mXIndex, mXSamples),
			Projection::stratified(rng.getFloat(), mYIndex, mYSamples), 0);
		mXIndex++;
		if (mXIndex >= mXSamples)
		{
			mXIndex = 0;
			mYIndex++;
		}
		return ret;
	}
}