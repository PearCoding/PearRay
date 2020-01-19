#pragma once

#include "Random.h"
#include "ray/RayPackage.h"
#include "renderer/RenderTileStatistics.h"

#include <atomic>

namespace PR {

class ICamera;
class RenderContext;
class ISampler;
class PR_LIB RenderTile {
public:
	RenderTile(const Point2i& start, const Point2i& end,
			   const RenderContext& context, uint32 index);
	~RenderTile();

	inline void incIteration() { ++mIterationCount; }
	inline void reset()
	{
		mPixelSamplesRendered = 0;
		mIterationCount		  = 0;
	}

	Ray constructCameraRay(const Point2i& p, uint32 sample);

	inline bool isWorking() const { return mWorking; }
	inline bool accuire() { return !mWorking.exchange(true); }
	inline void setWorking(bool b) { mWorking = b; }

	inline const Point2i& start() const { return mStart; }
	inline const Point2i& end() const { return mEnd; }
	inline const Size2i& viewSize() const { return mViewSize; }
	inline const Size2i& imageSize() const { return mImageSize; }
	inline uint32 index() const { return mIndex; }

	inline bool isFinished() const { return mPixelSamplesRendered >= mMaxPixelSamples; }
	inline uint32 maxPixelSamples() const { return mMaxPixelSamples; }
	inline uint32 pixelSamplesRendered() const { return mPixelSamplesRendered; }
	inline uint32 iterationCount() const { return mIterationCount; }

	inline Random& random() { return mRandom; }

	inline ISampler* aaSampler() const { return mAASampler.get(); }
	inline ISampler* lensSampler() const { return mLensSampler.get(); }
	inline ISampler* timeSampler() const { return mTimeSampler.get(); }

	inline const RenderTileStatistics& statistics() const { return mStatistics; }
	inline RenderTileStatistics& statistics() { return mStatistics; }

	inline const RenderContext* context() const { return mContext; }

private:
	std::atomic<bool> mWorking;

	const Point2i mStart;
	const Point2i mEnd;
	const Size2i mViewSize;
	const Size2i mImageSize;
	const uint32 mIndex;
	uint32 mMaxPixelSamples;

	std::atomic<uint32> mPixelSamplesRendered;
	std::atomic<uint32> mIterationCount;

	Random mRandom;
	std::shared_ptr<ISampler> mAASampler;
	std::shared_ptr<ISampler> mLensSampler;
	std::shared_ptr<ISampler> mTimeSampler;

	uint32 mAASampleCount;
	uint32 mLensSampleCount;
	uint32 mTimeSampleCount;

	// t = t'*alpha + beta
	float mTimeAlpha;
	float mTimeBeta;

	RenderTileStatistics mStatistics;

	const RenderContext* const mContext;
	const ICamera* const mCamera;
	
	float mWeight_Cache;
};
} // namespace PR
