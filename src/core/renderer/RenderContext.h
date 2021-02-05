#pragma once

#include "RenderRandomMap.h"
#include "RenderSettings.h"
#include "RenderStatistics.h"
#include "RenderStatus.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

namespace PR {
class HitStream;
class IEntity;
class IIntegrator;
class IMaterial;
class LightSampler;
class OutputSystem;
class RayStream;
class RenderThread;
class RenderTile;
class RenderTileMap;
class RenderTileSession;
class Scene;

/* Iteration Terminology:
 * A pass is a walk through all pixels,
 * An interation consists of multiple passes, often just one.
 */
struct PR_LIB_CORE RenderIteration {
	uint32 Iteration;
	uint32 Pass;
};

using RenderIterationCallback = std::function<void(const RenderIteration&)>;

class PR_LIB_CORE RenderContext {
	friend class RenderThread;

	PR_CLASS_NON_COPYABLE(RenderContext);

public:
	RenderContext(uint32 index, const Point2i& viewOffset, const Size2i& viewSize,
				  const std::shared_ptr<IIntegrator>& integrator,
				  const std::shared_ptr<Scene>& scene,
				  const RenderSettings& settings);
	virtual ~RenderContext();

	/// Index of the render context in a multiple image context (sliced image rendering)
	inline uint32 index() const { return mIndex; }
	/// Offset of the render to the original output image
	inline Point2i viewOffset() const { return mViewOffset; }
	/// ROI of the render to the original output image
	inline Size2i viewSize() const { return mViewSize; }

	/// Current tiles used for thread rendering. Its a matter to change!
	size_t tileCount() const;
	/// Maximum tile size used at the beginning. Current tiles may be smaller but never larger.
	Size2i maxTileSize() const;

	// rtx and rty are the initial render tiles
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 rtx, uint32 rty, int32 threads = 0);
	/// Request a stop but do not wait
	void requestStop();
	/// Request a soft stop, which stops after a whole iteration but do not wait
	void requestSoftStop();
	/// Request a stop and wait
	void stop();
	/// Call necessary callbacks to notify that the renderer ended
	void notifyEnd();

	/// Returns true if the renderer will stop at the next possible time
	inline bool isStopping() const { return mShouldStop; }
	/// Returns true if the renderer is finished and all threads are stopped
	bool isFinished() const;
	/// Will wait for all threads to finish
	void waitForFinish();

	/// Will request a full clear of the output buffers the next starting iteration (not pass)
	inline void requestOutputClear() { mOutputClearRequest = true; }

	/// Threads used for rendering. Will not change while running
	inline size_t threadCount() const { return mThreads.size(); }

	// Slow and only copies!
	std::vector<Rect2i> currentTiles() const;

	/// Returns current render iteration with current iteration and pass
	inline RenderIteration currentIteration() const { return RenderIteration{ mIncrementalCurrentIteration / mIntegratorPassCount, mIncrementalCurrentIteration % mIntegratorPassCount }; }

	/// Integrator used for rendering
	inline std::shared_ptr<IIntegrator> integrator() const { return mIntegrator; }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	RenderStatistics statistics() const;
	RenderStatus status() const;

	inline std::shared_ptr<OutputSystem> output() const { return mOutputSystem; }
	inline std::shared_ptr<Scene> scene() const { return mScene; }
	inline RenderRandomMap* randomMap() const { return mRandomMap.get(); }
	inline std::shared_ptr<LightSampler> lightSampler() const { return mLightSampler; }

	/// Set a callback called each start of iteration. The internal state of the callee is undefined
	inline void addIterationCallback(const RenderIterationCallback& clb) { mIterationCallbacks.push_back(clb); }

protected:
	RenderTile* getNextTile();

private:
	void requestInternalStop();
	void reset();
	void handleNextIteration();
	void optimizeTileMap();

	const uint32 mIndex;
	const Point2i mViewOffset;
	const Size2i mViewSize;

	const std::shared_ptr<Scene> mScene;
	const std::shared_ptr<OutputSystem> mOutputSystem;

	mutable std::mutex mTileMutex;
	std::unique_ptr<RenderTileMap> mTileMap;

	std::mutex mIterationMutex;
	std::condition_variable mIterationCondition;
	std::atomic<uint32> mThreadsWaitingForIteration;

	std::atomic<uint32> mIncrementalCurrentIteration; // A linear variant of iterations and passes
	std::vector<std::unique_ptr<RenderThread>> mThreads;

	const RenderSettings mRenderSettings;

	std::unique_ptr<RenderRandomMap> mRandomMap;
	std::shared_ptr<LightSampler> mLightSampler;

	const std::shared_ptr<IIntegrator> mIntegrator;
	uint32 mIntegratorPassCount; // Cache

	std::atomic<bool> mShouldStop;
	std::atomic<bool> mShouldSoftStop;
	std::atomic<bool> mOutputClearRequest;

	std::vector<RenderIterationCallback> mIterationCallbacks;
};
} // namespace PR
