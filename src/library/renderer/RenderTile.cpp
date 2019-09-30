#include "RenderTile.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
static std::unique_ptr<Sampler> createSampler(SamplerMode mode, Random& random, uint32 samples)
{
	switch (mode) {
	case SM_RANDOM:
		return std::make_unique<RandomSampler>(random);
	case SM_UNIFORM:
		return std::make_unique<UniformSampler>(random, samples);
	case SM_JITTER:
		return std::make_unique<StratifiedSampler>(random, samples);
	default:
	case SM_MULTI_JITTER:
		return std::make_unique<MultiJitteredSampler>(random, samples);
	case SM_HALTON_QMC:
		return std::make_unique<HaltonQMCSampler>(samples);
	}
}

RenderTile::RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey,
					   const RenderContext& context, uint32 index)
	: mWorking(false)
	, mSX(sx)
	, mSY(sy)
	, mEX(ex)
	, mEY(ey)
	, mWidth(ex - sx)
	, mHeight(ey - sy)
	, mFullWidth(context.settings().filmWidth)
	, mFullHeight(context.settings().filmHeight)
	, mIndex(index)
	, mMaxSamples(context.settings().samplesPerPixel() * mWidth * mHeight)
	, mSamplesRendered(0)
	, mRandom(context.settings().seed + index)
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mSpectralSampler(nullptr)
	, mAASampleCount(context.settings().aaSampleCount)
	, mLensSampleCount(context.settings().lensSampleCount)
	, mTimeSampleCount(context.settings().timeSampleCount)
	, mSpectralSampleCount(context.settings().spectralSampleCount)
	, mContext(context)
{
	PR_ASSERT(mWidth > 0, "Invalid tile width");
	PR_ASSERT(mHeight > 0, "Invalid tile height");

	mAASampler = createSampler(
		mContext.settings().aaSampler,
		mRandom, mAASampleCount);

	mLensSampler = createSampler(
		mContext.settings().lensSampler,
		mRandom, mLensSampleCount);

	mTimeSampler = createSampler(
		mContext.settings().timeSampler,
		mRandom, mTimeSampleCount);

	mSpectralSampler = createSampler(
		mContext.settings().spectralSampler,
		mRandom, mSpectralSampleCount);

	switch (mContext.settings().timeMappingMode) {
	default:
	case TMM_CENTER:
		mTimeAlpha = 1;
		mTimeBeta  = -0.5;
		break;
	case TMM_RIGHT:
		mTimeAlpha = 1;
		mTimeBeta  = 0;
		break;
	case TMM_LEFT:
		mTimeAlpha = -1;
		mTimeBeta  = 0;
		break;
	}

	const float f = mContext.settings().timeScale;
	mTimeAlpha *= f;
	mTimeBeta *= f;
}

RenderTile::~RenderTile()
{
}

void RenderTile::inc()
{
	mSamplesRendered++;
}

void RenderTile::reset()
{
	mSamplesRendered = 0;
}

Ray RenderTile::constructCameraRay(uint32 px, uint32 py, uint32 sample)
{
	statistics().addPixelSampleCount();

	const uint32 aasample		= sample / (mLensSampleCount * mTimeSampleCount * mSpectralSampleCount);
	const uint32 lenssample		= (sample % (mLensSampleCount * mTimeSampleCount * mSpectralSampleCount)) / (mTimeSampleCount * mSpectralSampleCount);
	const uint32 timesample		= (sample % (mTimeSampleCount * mSpectralSampleCount)) / mSpectralSampleCount;
	const uint32 spectralsample = sample % mSpectralSampleCount;

	const auto aa   = aaSampler()->generate2D(aasample);
	const auto lens = lensSampler()->generate2D(lenssample);
	const float t   = mTimeAlpha * timeSampler()->generate1D(timesample) + mTimeBeta;

	uint32 waveInd = spectralsample;
	if (mContext.settings().spectralProcessMode != SPM_LINEAR) {
		float specInd = spectralSampler()->generate1D(spectralsample);
		waveInd		  = std::min<uint32>(mContext.spectrumDescriptor()->samples() - 1,
									 std::floor(specInd
												* mContext.spectrumDescriptor()->samples()));
	}

	const float x = px + (aa(0) - 0.5f + mContext.offsetX());
	const float y = py + (aa(1) - 0.5f + mContext.offsetY());

	CameraSample cameraSample;
	cameraSample.SensorSize		 = Eigen::Vector2i(mFullWidth, mFullHeight);
	cameraSample.Pixel[0]		 = x;
	cameraSample.Pixel[1]		 = y;
	cameraSample.R[0]			 = lens(0);
	cameraSample.R[1]			 = lens(1);
	cameraSample.Time			 = t;
	cameraSample.WavelengthIndex = waveInd;

	return mContext.scene()->activeCamera()->constructRay(cameraSample);
}
} // namespace PR
