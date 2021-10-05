#include "RenderTile.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "camera/ICamera.h"
#include "math/Hash.h"
#include "sampler/ISampler.h"
#include "scene/Scene.h"
#include "spectral/ISpectralMapper.h"

namespace PR {
constexpr size_t SLOT_RND_PRIME = 4201321; // Just a random prime number to not have same randomizer as pixels

RenderTile::RenderTile(const Point2i& start, const Point2i& end,
					   RenderContext* context, const RenderTileContext& tileContext)
	: mStatus(static_cast<LockFreeAtomic::value_type>(RenderTileStatus::Idle))
	, mCurrentThread(nullptr)
	, mStart(start)
	, mEnd(end)
	, mViewSize(Size2i::fromPoints(start, end))
	, mImageSize(context->settings().filmWidth, context->settings().filmHeight)
	, mMaxIterationCount(context->settings().maxSampleCount())
	, mMaxPixelSamples(mViewSize.area() * mMaxIterationCount)
	, mContext(tileContext)
	, mWorkStart()
	, mLastWorkTime()
	, mRenderContext(context)
	, mRenderRandomMap(context->randomMap())
	, mCamera(context->scene()->activeCamera())
{
	PR_ASSERT(mViewSize.isValid(), "Invalid tile size");

	// Initialize
	for (size_t i = 0; i < mRandomSlots.size(); ++i) {
		mRandomSlots[i] = Random(context->settings().seed ^ (SLOT_RND_PRIME + i));
	}

	// Even while each sampler has his own number of requested samples...
	// each sampler deals with the combination of all requested samples
	mAASampler		 = mRenderContext->settings().createAASampler(random(RandomSlot::AA));
	mLensSampler	 = mRenderContext->settings().createLensSampler(random(RandomSlot::Lens));
	mTimeSampler	 = mRenderContext->settings().createTimeSampler(random(RandomSlot::Time));
	mSpectralSampler = mRenderContext->settings().createSpectralSampler(random(RandomSlot::Spectral));

	mSpectralMapper = mRenderContext->settings().createSpectralMapper("pixel", mRenderContext);

	switch (mRenderContext->settings().timeMappingMode) {
	default:
	case TimeMappingMode::Center:
		mTimeAlpha = 1;
		mTimeBeta  = -0.5;
		break;
	case TimeMappingMode::Right:
		mTimeAlpha = 1;
		mTimeBeta  = 0;
		break;
	case TimeMappingMode::Left:
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

std::optional<CameraRay> RenderTile::constructCameraRay(const Point2i& p, const RenderIteration& iter)
{
	PR_ASSERT(mStatus == (int)RenderTileStatus::Working, "Trying to use a tile which is not acquired");

	PR_PROFILE_THIS;

	statistics().add(RenderStatisticEntry::PixelSampleCount);
	++mContext.PixelSamplesRendered;
	const uint32 sample = iter.Iteration;

	Random& rnd = random(p);

	// Sample most information accesable by a camera
	CameraSample cameraSample;
	cameraSample.SensorSize	 = mImageSize;
	cameraSample.Pixel		 = (p + mRenderContext->viewOffset()).cast<float>() + mAASampler->generate2D(rnd, sample).array() - Point2f(0.5f, 0.5f);
	cameraSample.Lens		 = mLensSampler->generate2D(rnd, sample);
	cameraSample.Time		 = mTimeAlpha * mTimeSampler->generate1D(rnd, sample) + mTimeBeta;
	cameraSample.BlendWeight = 1.0f;
	cameraSample.Importance	 = 1.0f;

	// Sample wavelength
	if (mRenderContext->settings().spectralMono) {
		cameraSample.WavelengthNM  = SpectralBlob(mRenderContext->settings().spectralStart);
		cameraSample.WavelengthPDF = 1.0f;
	} else {
		SpectralSampleInput ssin(rnd); // TODO: Maybe use mSpectralSampler??
		ssin.Purpose = SpectralSamplePurpose::Pixel;
		ssin.Pixel	 = p + mRenderContext->viewOffset();

		SpectralSampleOutput ssout;
		mSpectralMapper->sample(ssin, ssout);

		cameraSample.WavelengthNM  = ssout.WavelengthNM;
		cameraSample.WavelengthPDF = ssout.PDF;
		cameraSample.BlendWeight *= ssout.BlendWeight;
	}

	// Construct actual ray
	std::optional<CameraRay> ray = mCamera->constructRay(cameraSample);
	if (PR_LIKELY(ray.has_value())) {
		if (PR_LIKELY(ray.value().BlendWeight <= 0.0f))
			ray.value().BlendWeight = cameraSample.BlendWeight;
		if (PR_LIKELY((ray.value().Importance <= 0.0f).any()))
			ray.value().Importance = cameraSample.Importance;
		if (PR_LIKELY((ray.value().WavelengthNM <= 0.0f).any()))
			ray.value().WavelengthNM = cameraSample.WavelengthNM;
		if (PR_LIKELY((ray.value().WavelengthPDF <= 0.0f).any()))
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

bool RenderTile::accuire(const RenderThread* thread)
{
	if (isFinished())
		return false;

	LockFreeAtomic::value_type expected = static_cast<LockFreeAtomic::value_type>(RenderTileStatus::Idle);
	const bool res						= mStatus.compare_exchange_strong(expected, static_cast<LockFreeAtomic::value_type>(RenderTileStatus::Working));

	if (res) {
		mCurrentThread = thread;
		mWorkStart	   = std::chrono::high_resolution_clock::now();
		return true;
	} else {
		return false;
	}
}

void RenderTile::release()
{
	LockFreeAtomic::value_type expected = static_cast<LockFreeAtomic::value_type>(RenderTileStatus::Working);
	const bool res						= mStatus.compare_exchange_strong(expected, static_cast<LockFreeAtomic::value_type>(RenderTileStatus::Done));

	if (res) {
		auto end	   = std::chrono::high_resolution_clock::now();
		mLastWorkTime  = std::chrono::duration_cast<std::chrono::microseconds>(end - mWorkStart);
		mCurrentThread = nullptr;
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
