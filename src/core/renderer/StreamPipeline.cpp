#include "StreamPipeline.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "camera/ICamera.h"
#include "math/Bits.h"

namespace PR {
StreamPipeline::StreamPipeline(RenderContext* ctx)
	: mContext(ctx)
	, mTile(nullptr)
	, mWriteRayStream(std::make_unique<RayStream>(ctx->settings().maxParallelRays))
	, mReadRayStream(std::make_unique<RayStream>(ctx->settings().maxParallelRays))
	, mHitStream(ctx->settings().maxParallelRays)
	, mGroupContainer()
	, mCurrentVirtualPixelIndex(0)
	, mCurrentPixelIndex(0)
	, mMaxPixelCount(0)
{
}

StreamPipeline::~StreamPipeline()
{
}

void StreamPipeline::reset(RenderTile* tile)
{
	mTile					  = tile;
	const Size2i size		  = mTile->viewSize();
	mMaxPixelCount			  = static_cast<size_t>(size.Width) * static_cast<size_t>(size.Height);
	mCurrentVirtualPixelIndex = 0;
	mCurrentPixelIndex		  = 0;

	mWriteRayStream->reset();
	mReadRayStream->reset();
	mHitStream.reset();
	mGroupContainer.reset();
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
	mHitStream.reset();
	mContext->scene()->traceRays(
		*mReadRayStream,
		mHitStream);

	// Early exit
	if (mContext->isStopping())
		return;

	// Setup hit stream
	mHitStream.setup(mContext->settings().sortHits);
}

void StreamPipeline::fillWithCameraRays()
{
	const Size2i size  = mTile->viewSize();
	const uint32 slice = mTile->imageSize().Width;

	const RenderIteration iter = mContext->currentIteration();
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

		std::optional<CameraRay> camera_ray = mTile->constructCameraRay(p, iter);
		if (camera_ray.has_value()) {
			uint32 grp_id;
			{
				RayGroup grp;
				grp.BlendWeight	  = camera_ray.value().BlendWeight;
				grp.Importance	  = camera_ray.value().Importance;
				grp.WavelengthNM  = camera_ray.value().WavelengthNM;
				grp.WavelengthPDF = camera_ray.value().WavelengthPDF;
				grp.Time		  = camera_ray.value().Time;
				grp.TimePDF		  = 1; // TODO: Support this?
				grp_id			  = mGroupContainer.registerGroup(std::move(grp));
			}

			Ray ray;
			ray.Origin		   = camera_ray.value().Origin;
			ray.Direction	   = camera_ray.value().Direction;
			ray.MaxT		   = camera_ray.value().MaxT;
			ray.MinT		   = camera_ray.value().MinT;
			ray.WavelengthNM   = camera_ray.value().WavelengthNM;
			ray.IterationDepth = 0;
			ray.GroupID		   = grp_id;
			ray.PixelIndex	   = p(1) * slice + p(0);
			ray.Flags		   = (camera_ray.value().IsMonochrome ? (uint32)RayFlag::Monochrome : 0) | (uint32)RayFlag::Camera;

			enqueueCameraRay(ray);
		}
		++mCurrentPixelIndex;
	}
}
} // namespace PR
