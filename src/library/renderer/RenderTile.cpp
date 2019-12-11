#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
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
	, mAASampleCount(context.settings().aaSampleCount)
	, mLensSampleCount(context.settings().lensSampleCount)
	, mTimeSampleCount(context.settings().timeSampleCount)
	, mContext(&context)
	, mCamera(context.scene()->activeCamera().get())
{
	PR_ASSERT(mWidth > 0, "Invalid tile width");
	PR_ASSERT(mHeight > 0, "Invalid tile height");

	mAASampler   = mContext->settings().createAASampler(mRandom);
	mLensSampler = mContext->settings().createLensSampler(mRandom);
	mTimeSampler = mContext->settings().createTimeSampler(mRandom);

	switch (mContext->settings().timeMappingMode) {
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

	const float f = mContext->settings().timeScale;
	mTimeAlpha *= f;
	mTimeBeta *= f;
}

RenderTile::~RenderTile()
{
}

Ray RenderTile::constructCameraRay(uint32 px, uint32 py, uint32 sample)
{
	PR_PROFILE_THIS;

	statistics().addPixelSampleCount();

	const uint32 timesample = sample % mTimeSampleCount;
	sample /= mTimeSampleCount;
	const uint32 lenssample = sample % mLensSampleCount;
	sample /= mLensSampleCount;
	const uint32 aasample = sample;

	Vector2f AA = mAASampler->generate2D(aasample);
	AA(0) += mContext->offsetX() - 0.5f;
	AA(1) += mContext->offsetY() - 0.5f;

	Vector2f Lens = mLensSampler->generate2D(lenssample);

	float Time = mTimeAlpha * mTimeSampler->generate1D(timesample) + mTimeBeta;

	CameraSample cameraSample;
	cameraSample.SensorSize		 = Vector2i(mFullWidth, mFullHeight);
	cameraSample.Pixel[0]		 = px + AA(0);
	cameraSample.Pixel[1]		 = py + AA(1);
	cameraSample.R[0]			 = Lens(0);
	cameraSample.R[1]			 = Lens(1);
	cameraSample.Time			 = Time;
	cameraSample.Weight			 = 1.0f / (mTimeSampleCount * mLensSampleCount * mAASampleCount);
	cameraSample.WavelengthIndex = 0;

	return mCamera->constructRay(cameraSample);
}
} // namespace PR
