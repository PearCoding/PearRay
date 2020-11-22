#pragma once

#include "RenderSettings.h"
#include "RenderStatus.h"
#include "RenderTileStatistics.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <vector>

namespace PR {
class HitStream;
class IEntity;
class IIntegrator;
class IMaterial;
class FrameBufferSystem;
class LightSampler;
class RayStream;
class RenderTileSession;
class RenderThread;
class RenderTile;
class RenderTileMap;
class Scene;

struct OutputSpectralEntry;

using RenderIterationCallback			= std::function<void(uint32)>;
using RenderOutputSpectralSplatCallback = std::function<void(const RenderThread*, const OutputSpectralEntry*, size_t)>;

class PR_LIB_CORE RenderContext {
	friend class RenderThread;

	PR_CLASS_NON_COPYABLE(RenderContext);

public:
	RenderContext(uint32 index, const Point2i& viewOffset, const Size2i& viewSize,
				  const std::shared_ptr<IIntegrator>& integrator,
				  const std::shared_ptr<Scene>& scene,
				  const RenderSettings& settings);
	virtual ~RenderContext();

	inline uint32 index() const { return mIndex; }
	inline Point2i viewOffset() const { return mViewOffset; }
	inline Size2i viewSize() const { return mViewSize; }

	size_t tileCount() const;
	Size2i maxTileSize() const;

	// rtx and rty are the initial render tiles
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 rtx, uint32 rty, int32 threads = 0);
	void requestStop(); // Request a stop but do not wait
	void stop();		// Request a stop and wait
	void notifyEnd();

	inline bool isStopping() const { return mShouldStop; }
	bool isFinished() const;
	void waitForFinish();

	inline size_t threadCount() const { return mThreads.size(); }

	// Slow and only copies!
	std::vector<Rect2i> currentTiles() const;

	inline std::shared_ptr<IIntegrator> integrator() const { return mIntegrator; }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	RenderTileStatistics statistics() const;
	RenderStatus status() const;

	inline std::shared_ptr<FrameBufferSystem> output() const { return mOutputMap; }
	inline std::shared_ptr<Scene> scene() const { return mScene; }
	inline std::shared_ptr<LightSampler> lightSampler() const { return mLightSampler; }

	// Set a callback called each start of iteration. The internal state of the callee is undefined
	inline void addIterationCallback(const RenderIterationCallback& clb) { mIterationCallbacks.push_back(clb); }

	inline void addOutputSpectralSplatCallback(const RenderOutputSpectralSplatCallback& clb) { mOutputSpectralSplatCallbacks.push_back(clb); }
	inline const std::vector<RenderOutputSpectralSplatCallback>& outputSpectralSplatCallbacks() const { return mOutputSpectralSplatCallbacks; }

protected:
	RenderTile* getNextTile();

private:
	void reset();
	void optimizeTileMap();

	const uint32 mIndex;
	const Point2i mViewOffset;
	const Size2i mViewSize;

	std::shared_ptr<Scene> mScene;
	std::shared_ptr<FrameBufferSystem> mOutputMap;

	mutable std::mutex mTileMutex;
	std::unique_ptr<RenderTileMap> mTileMap;

	std::mutex mIterationMutex;
	std::condition_variable mIterationCondition;
	std::atomic<uint32> mThreadsWaitingForIteration;

	std::atomic<uint32> mIncrementalCurrentIteration;
	std::list<RenderThread*> mThreads;

	const RenderSettings mRenderSettings;

	std::shared_ptr<LightSampler> mLightSampler;

	std::shared_ptr<IIntegrator> mIntegrator;
	bool mShouldStop;

	std::vector<RenderIterationCallback> mIterationCallbacks;
	std::vector<RenderOutputSpectralSplatCallback> mOutputSpectralSplatCallbacks;
};
} // namespace PR
