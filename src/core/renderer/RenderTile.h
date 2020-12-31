#pragma once

#include "Random.h"
#include "ray/Ray.h"
#include "renderer/RenderStatistics.h"

#include <atomic>
#include <chrono>
#include <optional>

namespace PR {
struct PR_LIB_CORE RenderTileContext {
	std::atomic<uint64> PixelSamplesRendered;
	RenderStatistics Statistics;

	inline RenderTileContext()
		: PixelSamplesRendered(0)
	{
	}

	inline RenderTileContext(const RenderTileContext& other)
		: PixelSamplesRendered(other.PixelSamplesRendered.load())
		, Statistics(other.Statistics)
	{
	}
};

class ICamera;
struct CameraRay;
class RenderContext;
struct RenderIteration;
class ISampler;
class ISpectralMapper;

enum RenderTileStatus {
	RTS_Idle	= 0,
	RTS_Working = 1,
	RTS_Done	= 2
};

class PR_LIB_CORE RenderTile {
public:
	RenderTile(const Point2i& start, const Point2i& end,
			   RenderContext* context, const RenderTileContext& tileContext = RenderTileContext());
	~RenderTile();

	inline void reset()
	{
		mContext.PixelSamplesRendered = 0;
		makeIdle();
	}

	std::optional<CameraRay> constructCameraRay(const Point2i& p, const RenderIteration& iter);

	inline RenderTileStatus status() const { return (RenderTileStatus)mStatus.load(); }
	inline bool isWorking() const { return mStatus == RTS_Working; }
	bool accuire();
	void release();
	inline void makeIdle() { mStatus = RTS_Idle; }

	inline const Point2i& start() const { return mStart; }
	inline const Point2i& end() const { return mEnd; }
	inline const Size2i& viewSize() const { return mViewSize; }
	inline const Size2i& imageSize() const { return mImageSize; }

	inline bool isFinished() const { return maxPixelSamples() != 0 /* Progressive */ && pixelSamplesRendered() >= maxPixelSamples(); }
	inline bool isMarkedDone() const { return mStatus == RTS_Done; } // Used by RenderContext to mark a full pass being done

	inline uint64 maxPixelSamples() const { return mMaxPixelSamples; }
	inline uint64 pixelSamplesRendered() const { return mContext.PixelSamplesRendered; }

	std::pair<std::unique_ptr<RenderTile>, std::unique_ptr<RenderTile>>
	split(int dim) const;

	inline Random& random() { return mRandom; }

	inline ISampler* aaSampler() const { return mAASampler.get(); }
	inline ISampler* lensSampler() const { return mLensSampler.get(); }
	inline ISampler* timeSampler() const { return mTimeSampler.get(); }
	inline ISampler* spectralSampler() const { return mSpectralSampler.get(); }

	inline ISpectralMapper* spectralMapper() const { return mSpectralMapper.get(); }

	inline const RenderStatistics& statistics() const { return mContext.Statistics; }
	inline RenderStatistics& statistics() { return mContext.Statistics; }

	inline const RenderContext* context() const { return mRenderContext; }

	inline std::chrono::microseconds lastWorkTime() const { return mLastWorkTime; }

private:
#if ATOMIC_INT_LOCK_FREE == 2
	using LockFreeAtomic = std::atomic<int>;
#elif ATOMIC_LONG_LOCK_FREE == 2
	using LockFreeAtomic = std::atomic<long>;
#elif ATOMIC_LLONG_LOCK_FREE == 2
	using LockFreeAtomic = std::atomic<long long>;
#elif ATOMIC_SHORT_LOCK_FREE == 2
	using LockFreeAtomic = std::atomic<short>;
#elif ATOMIC_CHAR_LOCK_FREE == 2
	using LockFreeAtomic = std::atomic<char>;
#else
	using LockFreeAtomic = std::atomic<int>;
#endif
	LockFreeAtomic mStatus;

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
	const std::shared_ptr<ICamera> mCamera;
};
} // namespace PR
