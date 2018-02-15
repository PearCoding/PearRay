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
	case SM_Random:
		return std::make_unique<RandomSampler>(random);
	case SM_Uniform:
		return std::make_unique<UniformSampler>(random, samples);
	case SM_Jitter:
		return std::make_unique<StratifiedSampler>(random, samples);
	default:
	case SM_MultiJitter:
		return std::make_unique<MultiJitteredSampler>(random, samples);
	case SM_HaltonQMC:
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
	, mIndex(index)
	, mMaxSamples(context.settings().maxCameraSampleCount() * mWidth * mHeight)
	, mSamplesRendered(0)
	, mRandom(context.settings().seed() + index)
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mSpectralSampler(nullptr)
	, mContext(context)
{
	PR_ASSERT(mWidth > 0, "Invalid tile width");
	PR_ASSERT(mHeight > 0, "Invalid tile height");

	mAASampler = createSampler(
		mContext.settings().aaSampler(), mRandom, mContext.settings().maxAASampleCount());

	mLensSampler = createSampler(
		mContext.settings().lensSampler(), mRandom, mContext.settings().maxLensSampleCount());

	mTimeSampler = createSampler(
		mContext.settings().timeSampler(), mRandom, mContext.settings().maxTimeSampleCount());

	mSpectralSampler = createSampler(
		mContext.settings().spectralSampler(), mRandom, mContext.settings().maxSpectralSampleCount());
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
	//const auto aaM = mRenderSettings.maxAASampleCount();
	const auto lensM	 = mContext.settings().maxLensSampleCount();
	const auto timeM	 = mContext.settings().maxTimeSampleCount();
	const auto spectralM = mContext.settings().maxSpectralSampleCount();

	const auto aasample		  = sample / (lensM * timeM * spectralM);
	const auto lenssample	 = (sample % (lensM * timeM * spectralM)) / (timeM * spectralM);
	const auto timesample	 = (sample % (timeM * spectralM)) / spectralM;
	const auto spectralsample = sample % spectralM;

	const auto aa   = aaSampler()->generate2D(aasample);
	const auto lens = lensSampler()->generate2D(lenssample);
	auto t			= timeSampler()->generate1D(timesample);

	switch (mContext.settings().timeMappingMode()) {
	default:
	case TMM_Center:
		t -= 0.5;
		break;
	case TMM_Right:
		break;
	case TMM_Left:
		t *= -1;
		break;
	}
	t *= mContext.settings().timeScale();

	uint8 specInd = std::min<uint8>(mContext.spectrumDescriptor()->samples() - 1,
									std::floor(
										spectralSampler()->generate1D(spectralsample) * mContext.spectrumDescriptor()->samples()));

	const float x = pixel(0) + aa(0) - 0.5f;
	const float y = pixel(1) + aa(1) - 0.5f;

	CameraSample cameraSample;
	cameraSample.SensorSize = Eigen::Vector2i(mContext.fullWidth(), mContext.fullHeight());
	cameraSample.PixelF		= Eigen::Vector2f(x + mContext.offsetX(), y + mContext.offsetY());
	cameraSample.Pixel		= Eigen::Vector2i(
		 std::min(std::max<uint32>(mContext.offsetX(), std::round(x)),
				  mContext.offsetX() + mContext.width() - 1),
		 std::min(std::max<uint32>(mContext.offsetY(), std::round(y)),
				  mContext.offsetY() + mContext.height() - 1));
	cameraSample.R				 = Eigen::Vector2f(lens(0), lens(1));
	cameraSample.Time			 = t;
	cameraSample.WavelengthIndex = specInd;

	return mContext.camera()->constructRay(cameraSample);
}
} // namespace PR
