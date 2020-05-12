#include "StreamPipeline.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"

namespace PR {
StreamPipeline::StreamPipeline(RenderContext* ctx)
	: mContext(ctx)
	, mTile(nullptr)
	, mCurrentPixelPos(Point2i::Zero())
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
	mTile = tile;
	mWriteRayStream->reset();
	mReadRayStream->reset();
	mHitStream->reset();
	mCurrentPixelPos = Point2i::Zero();
}

bool StreamPipeline::isFinished() const
{
	if (!mTile)
		return true;

	const Size2i size = mTile->viewSize();
	if (mCurrentPixelPos(1) < size.Height)
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
	if (mTile->context()->isStopping())
		return;

	// Swap buffers
	std::swap(mWriteRayStream, mReadRayStream);
	mWriteRayStream->reset();

	// Trace rays
	mHitStream->reset();
	mTile->context()->scene()->traceRays(
		*mReadRayStream,
		*mHitStream);

	// Early exit
	if (mTile->context()->isStopping())
		return;

	// Setup hit stream
	mHitStream->setup(mTile->context()->settings().sortHits);
}

void StreamPipeline::fillWithCameraRays()
{
	const Size2i size  = mTile->viewSize();
	const uint32 slice = mTile->imageSize().Width;

	bool forceBreak		   = false;
	const uint32 iterCount = mTile->iterationCount();
	for (; mCurrentPixelPos(1) < size.Height; ++mCurrentPixelPos(1)) {
		// Allow continuation
		if (mCurrentPixelPos(0) >= size.Width)
			mCurrentPixelPos(0) = 0;

		for (; mCurrentPixelPos(0) < size.Width; ++mCurrentPixelPos(0)) {
			if (mWriteRayStream->isFull() || mTile->context()->isStopping()) {
				forceBreak = true;
				break;
			}

			const Point2i p = mCurrentPixelPos + mTile->start();

			Ray ray		   = mTile->constructCameraRay(p, iterCount);
			ray.PixelIndex = p(1) * slice + p(0);

			enqueueCameraRay(ray);
		}

		if (forceBreak)
			break;
	}
}
} // namespace PR
