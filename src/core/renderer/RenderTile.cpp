#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/CIE.h"

//#define PR_SAMPLE_BY_CIE_Y
#define PR_SAMPLE_BY_CIE_XYZ

#if defined(PR_SAMPLE_BY_CIE_Y) || defined(PR_SAMPLE_BY_CIE_XYZ)
#define PR_SAMPLE_BY_CIE
#endif

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
	, mSpectralStart(context.settings().spectralStart)
	, mSpectralEnd(context.settings().spectralEnd)
	, mSpectralSpan(mSpectralEnd - mSpectralStart)
	, mSpectralDelta(mSpectralSpan / PR_SPECTRAL_BLOB_SIZE)
	, mSpectralMonotonic(context.settings().spectralMono)
	, mRenderContext(&context)
	, mCamera(context.scene()->activeCamera().get())
{
	PR_ASSERT(mViewSize.isValid(), "Invalid tile size");

	// Even while each sampler has his own number of requested samples...
	// each sampler deals with the combination of all requested samples
	mAASampler		 = mRenderContext->settings().createAASampler(mRandom);
	mLensSampler	 = mRenderContext->settings().createLensSampler(mRandom);
	mTimeSampler	 = mRenderContext->settings().createTimeSampler(mRandom);
	mSpectralSampler = mRenderContext->settings().createSpectralSampler(mRandom);

	mMaxIterationCount = mRenderContext->settings().maxSampleCount();
	mMaxPixelSamples *= mMaxIterationCount;

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

	mWeight_Cache = 1.0f / mMaxIterationCount;
}

RenderTile::~RenderTile()
{
}

/* Our sample approach gives each pixel in a single tile the SAME samples (except for random sampler)! 
 * This may sometimes good, but also bad... more investigations required...
 */
Ray RenderTile::constructCameraRay(const Point2i& p, uint32 sample)
{
	PR_PROFILE_THIS;

	statistics().addPixelSampleCount();
	++mContext.PixelSamplesRendered;

	CameraSample cameraSample;
	cameraSample.SensorSize = mImageSize;
	cameraSample.Pixel		= (p + mRenderContext->viewOffset()).cast<float>() + mAASampler->generate2D(sample).array() - Point2f(0.5f, 0.5f);
	cameraSample.Lens		= mLensSampler->generate2D(sample);
	cameraSample.Time		= mTimeAlpha * mTimeSampler->generate1D(sample) + mTimeBeta;
	cameraSample.Weight		= mWeight_Cache;

	// Sample wavelength
	if (mSpectralMonotonic) {
		cameraSample.WavelengthNM = SpectralBlob(mSpectralStart);
	} else {
#if defined(PR_SAMPLE_BY_CIE)
		if (mSpectralStart == PR_CIE_WAVELENGTH_START && mSpectralEnd == PR_CIE_WAVELENGTH_END) {
			float u = mSpectralSampler->generate1D(sample);
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
				float pdf;
#ifdef PR_SAMPLE_BY_CIE_Y
				cameraSample.WavelengthNM(i) = CIE::sample_y(k, pdf);
#else
				cameraSample.WavelengthNM(i) = CIE::sample_xyz(k, pdf);
#endif
				cameraSample.Weight(i) /= pdf;
			}
		} else {
#endif
			float start					 = mSpectralSampler->generate1D(sample) * mSpectralSpan; // Wavelength inside the span
			cameraSample.WavelengthNM(0) = start + mSpectralStart;								 // Hero wavelength
			PR_OPT_LOOP
			for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				cameraSample.WavelengthNM(i) = mSpectralStart + std::fmod(start + i * mSpectralDelta, mSpectralSpan);
#if defined(PR_SAMPLE_BY_CIE)
		}
#endif
	}

	return mCamera->constructRay(cameraSample);
}

bool RenderTile::accuire()
{
	if (!isFinished() && !mWorking.exchange(true)) {
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
	Point2i endLeft	  = end();
	endLeft(dim)	  = startLeft(dim) + viewSize().asArray()(dim) / 2;

	Point2i startRight = start();
	Point2i endRight   = end();
	startRight(dim)	   = endLeft(dim);

	// Construct context of the two tiles
	RenderTileContext left, right;
	left.IterationCount	 = mContext.IterationCount.load();
	right.IterationCount = mContext.IterationCount.load();

	left.PixelSamplesRendered = mContext.PixelSamplesRendered / 2;
	// Sometimes PixelSamplesRendered is not a multiply of 2 -> Fix it
	right.PixelSamplesRendered = mContext.PixelSamplesRendered / 2 + mContext.PixelSamplesRendered % 2;

	left.Statistics	 = mContext.Statistics.half();
	right.Statistics = left.Statistics;

	// Create tiles
	auto leftTile  = new RenderTile(startLeft, endLeft, *mRenderContext, left);
	auto rightTile = new RenderTile(startRight, endRight, *mRenderContext, right);
	return std::make_pair(leftTile, rightTile);
}
} // namespace PR
