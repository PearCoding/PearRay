#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderTile::RenderTile(const Point2i& start, const Point2i& end,
					   const RenderContext& context, uint32 index)
	: mWorking(false)
	, mStart(start)
	, mEnd(end)
	, mViewSize(Size2i::fromPoints(start, end))
	, mImageSize(context.settings().filmWidth, context.settings().filmHeight)
	, mIndex(index)
	, mMaxPixelSamples(mViewSize.area())
	, mPixelSamplesRendered(0)
	, mIterationCount(0)
	, mRandom(context.settings().seed + index)
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mAASampleCount(0)
	, mLensSampleCount(0)
	, mTimeSampleCount(0)
	, mContext(&context)
	, mCamera(context.scene()->activeCamera().get())
{
	PR_ASSERT(mViewSize.isValid(), "Invalid tile size");

	mAASampler   = mContext->settings().createAASampler(mRandom);
	mLensSampler = mContext->settings().createLensSampler(mRandom);
	mTimeSampler = mContext->settings().createTimeSampler(mRandom);

	mAASampleCount   = mAASampler->maxSamples();
	mLensSampleCount = mLensSampler->maxSamples();
	mTimeSampleCount = mTimeSampler->maxSamples();
	mMaxPixelSamples *= mAASampleCount * mLensSampleCount * mTimeSampleCount;

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

	mWeight_Cache = 1.0f / (mTimeSampleCount * mLensSampleCount * mAASampleCount);
}

RenderTile::~RenderTile()
{
}

Ray RenderTile::constructCameraRay(const Point2i& p, uint32 sample)
{
	PR_PROFILE_THIS;

	statistics().addPixelSampleCount();
	++mPixelSamplesRendered;

	const uint32 timesample = sample % mTimeSampleCount;
	sample /= mTimeSampleCount;
	const uint32 lenssample = sample % mLensSampleCount;
	sample /= mLensSampleCount;
	const uint32 aasample = sample;

	Point2f AA = mAASampler->generate2D(aasample);
	AA += mContext->viewOffset().cast<float>() - Point2f(0.5f, 0.5f);

	Point2f Lens = mLensSampler->generate2D(lenssample);

	float Time = mTimeAlpha * mTimeSampler->generate1D(timesample) + mTimeBeta;

	CameraSample cameraSample;
	cameraSample.SensorSize		 = mImageSize;
	cameraSample.Pixel			 = p.cast<float>() + AA;
	cameraSample.Lens			 = Lens;
	cameraSample.Time			 = Time;
	cameraSample.Weight			 = mWeight_Cache;
	cameraSample.WavelengthIndex = 0;

	return mCamera->constructRay(cameraSample);
}
} // namespace PR
