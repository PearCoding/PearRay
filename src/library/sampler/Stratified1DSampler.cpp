#include "Stratified1DSampler.h"
#include "Projection.h"

namespace PR
{
	Stratified1DSampler::Stratified1DSampler(uint32 samples) :
		Sampler(), mSamples(samples), mIndex(0)
	{

	}

	PM::vec Stratified1DSampler::generate(Random& rng)
	{
		auto ret = PM::pm_Set(Projection::stratified(rng.getFloat(), mIndex, mSamples), 0, 0);
		mIndex++;
		return ret;
	}
}