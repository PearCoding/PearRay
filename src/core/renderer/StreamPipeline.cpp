#include "StreamPipeline.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "math/Bits.h"

namespace PR {
StreamPipeline::StreamPipeline(RenderContext* ctx)
	: mContext(ctx)
	, mTile(nullptr)
	, mCurrentVirtualPixelIndex(0)
	, mCurrentPixelIndex(0)
	, mMaxPixelCount(0)
{
	const auto initialSize = ctx->settings().maxParallelRays;
	mWriteRayStream		   = std::make_unique<RayStream>(initialSize);
	mReadRayStream		   = std::make_unique<RayStream>(initialSize);
	mHitStream			   = std::make_unique<HitStream>(initialSize);
}

StreamPipeline::~StreamPipeline()
{
}

void StreamPipeline::reset(RenderTile* tile)
{
	mTile					  = tile;
	const Size2i size		  = mTile->viewSize();
	mMaxPixelCount			  = size.Width * size.Height;
	mCurrentVirtualPixelIndex = 0;
	mCurrentPixelIndex		  = 0;

	mWriteRayStream->reset();
	mReadRayStream->reset();
	mHitStream->reset();
}

bool StreamPipeline::isFinished() const
{
	if (!mTile || mContext->isStopping())
		return true;

	if (mCurrentPixelIndex < mMaxPixelCount)
		return false;

	return mReadRayStream->isEmpty();
}

void StreamPipeline::runPipeline()
{
	if (!mTile)
		return;

	PR_PROFILE_THIS;

	// Fill write ray stream with camera rays till full
	fillWithCameraRays();

	// Early exit
	if (mContext->isStopping())
		return;

	// Swap buffers
	std::swap(mWriteRayStream, mReadRayStream);
	mWriteRayStream->reset();

	// Trace rays
	mHitStream->reset();
	mContext->scene()->traceRays(
		*mReadRayStream,
		*mHitStream);

	// Early exit
	if (mContext->isStopping())
		return;

	// Setup hit stream
	mHitStream->setup(mContext->settings().sortHits);
}

void StreamPipeline::fillWithCameraRays()
{
	const Size2i size  = mTile->viewSize();
	const uint32 slice = mTile->imageSize().Width;

	const uint32 iterCount = mTile->iterationCount();
	while (mCurrentPixelIndex < mMaxPixelCount) {
		if (mWriteRayStream->isFull() || mContext->isStopping())
			break;

		uint32 x, y;

		morton_2_xy(mCurrentVirtualPixelIndex, x, y);
		++mCurrentVirtualPixelIndex;

		// Morton works on quad areas. We use garbage pixels for that and ignore them here
		if (x >= (uint32)size.Width || y >= (uint32)size.Height)
			continue;

		const Point2i p = Point2i(x, y) + mTile->start();

		Ray ray		   = mTile->constructCameraRay(p, iterCount);
		ray.PixelIndex = p(1) * slice + p(0);

		enqueueCameraRay(ray);
		++mCurrentPixelIndex;
	}
}
} // namespace PR
