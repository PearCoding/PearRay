#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/ISpectralMapper.h"

namespace PR {
RenderTile::RenderTile(const Point2i& start, const Point2i& end,
					   RenderContext* context, const RenderTileContext& tileContext)
	: mStatus(RTS_Idle)
	, mStart(start)
	, mEnd(end)
	, mViewSize(Size2i::fromPoints(start, end))
	, mImageSize(context->settings().filmWidth, context->settings().filmHeight)
	, mMaxIterationCount(context->settings().maxSampleCount())
	, mMaxPixelSamples(mViewSize.area() * mMaxIterationCount)
	, mContext(tileContext)
	, mWorkStart()
	, mLastWorkTime()
	, mRandom(context->settings().seed + start(0) + start(1))
	, mRenderContext(context)
	, mCamera(context->scene()->activeCamera().get())
{
	PR_ASSERT(mViewSize.isValid(), "Invalid tile size");

	// Even while each sampler has his own number of requested samples...
	// each sampler deals with the combination of all requested samples
	mAASampler = mRenderContext->settings().createAASampler(mRandom);
	mLensSampler = mRenderContext->settings().createLensSampler(mRandom);
	mTimeSampler = mRenderContext->settings().createTimeSampler(mRandom);
	mSpectralSampler = mRenderContext->settings().createSpectralSampler(mRandom);

	mSpectralMapper = mRenderContext->settings().createSpectralMapper(mRenderContext);

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
}

RenderTile::~RenderTile()
{
}

// TODO
/* Our sample approach gives each pixel in a single tile the SAME samples (except for random sampler)! 
 * This may sometimes good, but also bad... more investigations required...
 */
std::optional<CameraRay> RenderTile::constructCameraRay(const Point2i& p, const RenderIteration& iter)
{
	PR_ASSERT(mStatus == RTS_Working, "Trying to use a tile which is not acquired");

	PR_PROFILE_THIS;

	statistics().add(RST_PixelSampleCount);
	++mContext.PixelSamplesRendered;
	const uint32 sample = iter.Iteration;

	// Sample most information accesable by a camera
	CameraSample cameraSample;
	cameraSample.SensorSize	 = mImageSize;
	cameraSample.Pixel		 = (p + mRenderContext->viewOffset()).cast<float>() + mAASampler->generate2D(sample).array() - Point2f(0.5f, 0.5f);
	cameraSample.Lens		 = mLensSampler->generate2D(sample);
	cameraSample.Time		 = mTimeAlpha * mTimeSampler->generate1D(sample) + mTimeBeta;
	cameraSample.BlendWeight = 1.0f;
	cameraSample.Importance	 = 1.0f;

	// Sample wavelength
	if (mRenderContext->settings().spectralMono) {
		cameraSample.WavelengthNM  = SpectralBlob(mRenderContext->settings().spectralStart);
		cameraSample.WavelengthPDF = 1.0f;
	} else {
		const auto specSample	   = mSpectralMapper->sample(p + mRenderContext->viewOffset(), mSpectralSampler->generate1D(sample));
		cameraSample.WavelengthNM  = specSample.WavelengthNM;
		cameraSample.WavelengthPDF = specSample.PDF;
		cameraSample.BlendWeight *= specSample.BlendWeight;
	}

	// Construct actual ray
	std::optional<CameraRay> ray = mCamera->constructRay(cameraSample);
	if (PR_LIKELY(ray.has_value())) {
		if (PR_LIKELY(ray.value().BlendWeight.isZero()))
			ray.value().BlendWeight = cameraSample.BlendWeight;
		if (PR_LIKELY(ray.value().Importance.isZero()))
			ray.value().Importance = cameraSample.Importance;
		if (PR_LIKELY(ray.value().WavelengthNM.isZero()))
			ray.value().WavelengthNM = cameraSample.WavelengthNM;
		if (PR_LIKELY(ray.value().WavelengthPDF.isZero()))
			ray.value().WavelengthPDF = cameraSample.WavelengthPDF;
		if (PR_LIKELY(ray.value().Time <= 0.0f))
			ray.value().Time = cameraSample.Time;

		// Set monochrome by force if necessary
		if (mRenderContext->settings().spectralMono || !mRenderContext->settings().spectralHero)
			ray.value().IsMonochrome = true;

		if (ray.value().IsMonochrome)
			ray.value().Importance *= SpectralBlobUtils::HeroOnly();
	}

	return ray;
}

bool RenderTile::accuire()
{
	if (isFinished())
		return false;

	LockFreeAtomic::value_type expected = RTS_Idle;
	const bool res						= mStatus.compare_exchange_strong(expected, RTS_Working);

	if (res) {
		mWorkStart = std::chrono::high_resolution_clock::now();
		return true;
	} else {
		return false;
	}
}

void RenderTile::release()
{
	LockFreeAtomic::value_type expected = RTS_Working;
	const bool res						= mStatus.compare_exchange_strong(expected, RTS_Done);

	if (res) {
		auto end	  = std::chrono::high_resolution_clock::now();
		mLastWorkTime = std::chrono::duration_cast<std::chrono::microseconds>(end - mWorkStart);
	}
}

std::pair<std::unique_ptr<RenderTile>, std::unique_ptr<RenderTile>>
RenderTile::split(int dim) const
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
	left.PixelSamplesRendered = mContext.PixelSamplesRendered / 2;
	// Sometimes PixelSamplesRendered is not a multiply of 2 -> Fix it
	right.PixelSamplesRendered = mContext.PixelSamplesRendered / 2 + mContext.PixelSamplesRendered % 2;

	left.Statistics	 = mContext.Statistics.half();
	right.Statistics = left.Statistics;

	// Create tiles
	return std::make_pair(std::make_unique<RenderTile>(startLeft, endLeft, mRenderContext, left),
						  std::make_unique<RenderTile>(startRight, endRight, mRenderContext, right));
}
} // namespace PR
