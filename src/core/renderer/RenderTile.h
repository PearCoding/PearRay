#pragma once

#include "Random.h"
#include "ray/Ray.h"
#include "renderer/RenderTileStatistics.h"

#include <atomic>
#include <chrono>

namespace PR {
struct PR_LIB_CORE RenderTileContext {
	std::atomic<uint64> PixelSamplesRendered;
	std::atomic<uint32> IterationCount;
	RenderTileStatistics Statistics;

	inline RenderTileContext()
		: PixelSamplesRendered(0)
		, IterationCount(0)
	{
	}

	inline RenderTileContext(const RenderTileContext& other)
		: PixelSamplesRendered(other.PixelSamplesRendered.load())
		, IterationCount(other.IterationCount.load())
		, Statistics(other.Statistics)
	{
	}
};

class ICamera;
struct CameraRay;
class RenderContext;
class ISampler;
class ISpectralMapper;

class PR_LIB_CORE RenderTile {
public:
	RenderTile(const Point2i& start, const Point2i& end,
			   RenderContext* context, const RenderTileContext& tileContext = RenderTileContext());
	~RenderTile();

	inline void incIteration() { ++mContext.IterationCount; }
	inline void reset()
	{
		mContext.PixelSamplesRendered = 0;
		mContext.IterationCount		  = 0;
	}

	std::optional<CameraRay> constructCameraRay(const Point2i& p, uint32 sample);

	inline bool isWorking() const { return mWorking; }
	bool accuire();
	void release();

	inline const Point2i& start() const { return mStart; }
	inline const Point2i& end() const { return mEnd; }
	inline const Size2i& viewSize() const { return mViewSize; }
	inline const Size2i& imageSize() const { return mImageSize; }

	inline bool isFinished() const { return pixelSamplesRendered() >= maxPixelSamples(); }
	inline uint64 maxPixelSamples() const { return mMaxPixelSamples; }
	inline uint64 pixelSamplesRendered() const { return mContext.PixelSamplesRendered; }
	inline uint32 iterationCount() const { return mContext.IterationCount; }

	std::pair<RenderTile*, RenderTile*> split(int dim) const;

	inline Random& random() { return mRandom; }

	inline ISampler* aaSampler() const { return mAASampler.get(); }
	inline ISampler* lensSampler() const { return mLensSampler.get(); }
	inline ISampler* timeSampler() const { return mTimeSampler.get(); }
	inline ISampler* spectralSampler() const { return mSpectralSampler.get(); }

	inline ISpectralMapper* spectralMapper() const { return mSpectralMapper.get(); }

	inline const RenderTileStatistics& statistics() const { return mContext.Statistics; }
	inline RenderTileStatistics& statistics() { return mContext.Statistics; }

	inline const RenderContext* context() const { return mRenderContext; }

	inline std::chrono::microseconds lastWorkTime() const { return mLastWorkTime; }

private:
	std::atomic<bool> mWorking;

	const Point2i mStart;
	const Point2i mEnd;
	const Size2i mViewSize;
	const Size2i mImageSize;
	const uint64 mMaxIterationCount;
	const uint32 mMaxPixelSamples;

	RenderTileContext mContext;
	std::chrono::high_resolution_clock::time_point mWorkStart;
	std::chrono::microseconds mLastWorkTime;

	Random mRandom;
	std::shared_ptr<ISampler> mAASampler;
	std::shared_ptr<ISampler> mLensSampler;
	std::shared_ptr<ISampler> mTimeSampler;
	std::shared_ptr<ISampler> mSpectralSampler;

	std::shared_ptr<ISpectralMapper> mSpectralMapper;

	// t = t'*alpha + beta
	float mTimeAlpha;
	float mTimeBeta;

	RenderContext* mRenderContext;
	const ICamera* const mCamera;
};
} // namespace PR
