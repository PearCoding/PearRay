#include "RenderTile.h"
#include "RenderContext.h"
#include "camera/Camera.h"
#include "shader/ShaderClosure.h"

#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"

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
	, mFullWidth(context.settings().filmWidth())
	, mFullHeight(context.settings().filmHeight())
	, mIndex(index)
	, mMaxSamples(context.settings().samplesPerPixel() * mWidth * mHeight)
	, mSamplesRendered(0)
	, mRandom(context.settings().seed() + index)
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mSpectralSampler(nullptr)
	, mAASampleCount(context.settings().aaSampleCount())
	, mLensSampleCount(context.settings().lensSampleCount())
	, mTimeSampleCount(context.settings().timeSampleCount())
	, mSpectralSampleCount(context.settings().spectralSampleCount())
	, mContext(context)
{
	PR_ASSERT(mWidth > 0, "Invalid tile width");
	PR_ASSERT(mHeight > 0, "Invalid tile height");

	mAASampler = createSampler(
		mContext.settings().aaSampler(),
		mRandom, mAASampleCount);

	mLensSampler = createSampler(
		mContext.settings().lensSampler(),
		mRandom, mLensSampleCount);

	mTimeSampler = createSampler(
		mContext.settings().timeSampler(),
		mRandom, mTimeSampleCount);

	mSpectralSampler = createSampler(
		mContext.settings().spectralSampler(),
		mRandom, mSpectralSampleCount);

	switch (mContext.settings().timeMappingMode()) {
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

	const float f = mContext.settings().timeScale();
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

Ray RenderTile::constructCameraRay(const Eigen::Vector2i& pixel, uint32 sample)
{
	statistics().incPixelSampleCount();

	ShaderClosure sc;
	const auto aasample		  = sample / (mLensSampleCount * mTimeSampleCount * mSpectralSampleCount);
	const auto lenssample	 = (sample % (mLensSampleCount * mTimeSampleCount * mSpectralSampleCount)) / (mTimeSampleCount * mSpectralSampleCount);
	const auto timesample	 = (sample % (mTimeSampleCount * mSpectralSampleCount)) / mSpectralSampleCount;
	const auto spectralsample = sample % mSpectralSampleCount;

	const auto aa   = aaSampler()->generate2D(aasample);
	const auto lens = lensSampler()->generate2D(lenssample);
	auto t			= mTimeAlpha * timeSampler()->generate1D(timesample) + mTimeBeta;

	uint8 specInd = std::min<uint8>(mContext.spectrumDescriptor()->samples() - 1,
									std::floor(
										spectralSampler()->generate1D(spectralsample) * mContext.spectrumDescriptor()->samples()));

	const float x = pixel(0) + aa(0) - 0.5f;
	const float y = pixel(1) + aa(1) - 0.5f;

	CameraSample cameraSample;
	cameraSample.SensorSize = Eigen::Vector2i(mFullWidth, mFullHeight);
	cameraSample.PixelF		= Eigen::Vector2f(x + mContext.offsetX(), y + mContext.offsetY());
	cameraSample.Pixel		= Eigen::Vector2i(
		std::min(std::max<uint32>(mContext.offsetX(), std::round(cameraSample.PixelF(0))),
				 mContext.offsetX() + mContext.width() - 1),
		std::min(std::max<uint32>(mContext.offsetY(), std::round(cameraSample.PixelF(1))),
				 mContext.offsetY() + mContext.height() - 1));
	cameraSample.R				 = Eigen::Vector2f(lens(0), lens(1));
	cameraSample.Time			 = t;
	cameraSample.WavelengthIndex = specInd;

	return mContext.camera()->constructRay(cameraSample);
}
} // namespace PR
