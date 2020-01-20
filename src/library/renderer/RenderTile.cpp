#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderTile::RenderTile(const Point2i& start, const Point2i& end,
					   const RenderContext& context, const RenderTileContext& tileContext)
	: mWorking(false)
	, mStart(start)
	, mEnd(end)
	, mViewSize(Size2i::fromPoints(start, end))
	, mImageSize(context.settings().filmWidth, context.settings().filmHeight)
	, mMaxPixelSamples(mViewSize.area())
	, mContext(tileContext)
	, mWorkStart()
	, mLastWorkTime()
	, mRandom(context.settings().seed + (start(0) ^ start(1)))
	, mAASampler(nullptr)
	, mLensSampler(nullptr)
	, mTimeSampler(nullptr)
	, mAASampleCount(0)
	, mLensSampleCount(0)
	, mTimeSampleCount(0)
	, mRenderContext(&context)
	, mCamera(context.scene()->activeCamera().get())
{
	PR_ASSERT(mViewSize.isValid(), "Invalid tile size");

	mAASampler   = mRenderContext->settings().createAASampler(mRandom);
	mLensSampler = mRenderContext->settings().createLensSampler(mRandom);
	mTimeSampler = mRenderContext->settings().createTimeSampler(mRandom);

	mAASampleCount   = mAASampler->maxSamples();
	mLensSampleCount = mLensSampler->maxSamples();
	mTimeSampleCount = mTimeSampler->maxSamples();
	mMaxPixelSamples *= mAASampleCount * mLensSampleCount * mTimeSampleCount;

	switch (mRenderContext->settings().timeMappingMode) {
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

	const float f = mRenderContext->settings().timeScale;
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
	++mContext.PixelSamplesRendered;

	const uint32 timesample = sample % mTimeSampleCount;
	sample /= mTimeSampleCount;
	const uint32 lenssample = sample % mLensSampleCount;
	sample /= mLensSampleCount;
	const uint32 aasample = sample;

	Point2f AA = mAASampler->generate2D(aasample);
	AA += mRenderContext->viewOffset().cast<float>() - Point2f(0.5f, 0.5f);

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

bool RenderTile::accuire()
{
	if (!mWorking.exchange(true)) {
		mWorkStart = std::chrono::high_resolution_clock::now();
		return true;
	} else {
		return false;
	}
}

void RenderTile::release()
{
	if (mWorking.exchange(false)) {
		auto end	  = std::chrono::high_resolution_clock::now();
		mLastWorkTime = std::chrono::duration_cast<std::chrono::microseconds>(end - mWorkStart);
	}
}

std::pair<RenderTile*, RenderTile*> RenderTile::split(int dim) const
{
	// Split view size in dim at half
	Point2i startLeft = start();
	Point2i endLeft   = end();
	endLeft(dim)	  = startLeft(dim) + viewSize().asArray()(dim) / 2;

	Point2i startRight = start();
	Point2i endRight   = end();
	startRight(dim)	= endLeft(dim);

	// Construct context of the two tiles
	RenderTileContext left, right;
	left.IterationCount  = mContext.IterationCount.load();
	right.IterationCount = mContext.IterationCount.load();

	left.PixelSamplesRendered  = mContext.PixelSamplesRendered / 2;
	// Sometimes PixelSamplesRendered is not a multiply of 2 -> Fix it
	right.PixelSamplesRendered = mContext.PixelSamplesRendered / 2 + mContext.PixelSamplesRendered % 2;

	left.Statistics  = mContext.Statistics.half();
	right.Statistics = left.Statistics;

	// Create tiles
	auto leftTile  = new RenderTile(startLeft, endLeft, *mRenderContext, left);
	auto rightTile = new RenderTile(startRight, endRight, *mRenderContext, right);
	return std::make_pair(leftTile, rightTile);
}
} // namespace PR
