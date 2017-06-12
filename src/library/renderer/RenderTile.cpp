#include "RenderTile.h"
#include "RenderSettings.h"

#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"

namespace PR {
static Sampler* createSampler(SamplerMode mode, Random& random, uint32 samples)
{
	switch (mode) {
	case SM_Random:
		return new RandomSampler(random);
	case SM_Uniform:
		return new UniformSampler(random, samples);
	case SM_Jitter:
		return new StratifiedSampler(random, samples);
	default:
	case SM_MultiJitter:
		return new MultiJitteredSampler(random, samples);
	case SM_HaltonQMC:
		return new HaltonQMCSampler(samples);
	}
}

RenderTile::RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey,
					   const RenderSettings& settings, uint32 index)
	: mWorking(false)
	, mSX(sx)
	, mSY(sy)
	, mEX(ex)
	, mEY(ey)
	, mWidth(ex - sx)
	, mHeight(ey - sy)
	, mIndex(index)
	, mMaxSamples(settings.maxCameraSampleCount() * mWidth * mHeight)
	, mSamplesRendered(0)
	, mRandom(settings.seed() + index)
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mSpectralSampler(nullptr)
{
	PR_ASSERT(mWidth > 0, "Invalid tile width");
	PR_ASSERT(mHeight > 0, "Invalid tile height");

	mAASampler = createSampler(
		settings.aaSampler(), mRandom, settings.maxAASampleCount());

	mLensSampler = createSampler(
		settings.lensSampler(), mRandom, settings.maxLensSampleCount());

	mTimeSampler = createSampler(
		settings.timeSampler(), mRandom, settings.maxTimeSampleCount());

	mSpectralSampler = createSampler(
		settings.spectralSampler(), mRandom, settings.maxSpectralSampleCount());
}

RenderTile::~RenderTile()
{
	if (mAASampler)
		delete mAASampler;
	if (mLensSampler)
		delete mLensSampler;
	if (mTimeSampler)
		delete mTimeSampler;
	if (mSpectralSampler)
		delete mSpectralSampler;
}

void RenderTile::inc()
{
	mSamplesRendered++;
}

void RenderTile::reset()
{
	mSamplesRendered = 0;
}
}
