#pragma once

#include "RenderSettings.h"
#include "RenderStatus.h"
#include "RenderTileStatistics.h"

#include <condition_variable>
#include <list>
#include <mutex>
#include <vector>

namespace PR {
class HitStream;
class IEntity;
class IIntegrator;
class IMaterial;
class FrameBufferSystem;
class RayStream;
class RenderTileSession;
class RenderThread;
class RenderTile;
class RenderTileMap;
class Scene;

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
	inline const Point2i viewOffset() const { return mViewOffset; }
	inline const Size2i& viewSize() const { return mViewSize; }

	size_t tileCount() const;
	const Size2i& maxTileSize() const;

	// rtx and rty are the initial render tiles
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 rtx, uint32 rty, int32 threads = 0);
	void stop();
	void notifyEnd();

	inline bool isStopping() const { return mShouldStop; }
	bool isFinished() const;
	void waitForFinish();

	size_t threads() const { return mThreads.size(); }

	// Slow and only copies!
	std::vector<Rect2i> currentTiles() const;

	std::shared_ptr<IIntegrator> integrator() const { return mIntegrator; }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	// Light
	inline const std::vector<std::shared_ptr<IEntity>>& lights() const { return mLights; }
	inline float emissiveSurfaceArea() const { return mEmissiveSurfaceArea; }

	RenderTileStatistics statistics() const;
	RenderStatus status() const;

	inline std::shared_ptr<FrameBufferSystem> output() const { return mOutputMap; }
	inline std::shared_ptr<Scene> scene() const { return mScene; }

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

	std::vector<std::shared_ptr<IEntity>> mLights;
	float mEmissiveSurfaceArea;

	mutable std::mutex mTileMutex;
	std::unique_ptr<RenderTileMap> mTileMap;

	std::mutex mIterationMutex;
	std::condition_variable mIterationCondition;
	uint32 mThreadsWaitingForIteration;

	uint32 mIncrementalCurrentIteration;
	std::list<RenderThread*> mThreads;

	const RenderSettings mRenderSettings;

	std::shared_ptr<IIntegrator> mIntegrator;
	bool mShouldStop;
};
} // namespace PR
