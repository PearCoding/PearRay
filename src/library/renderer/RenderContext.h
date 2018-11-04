#pragma once

#include "RenderSettings.h"
#include "RenderStatistics.h"
#include "RenderStatus.h"

#include <condition_variable>
#include <list>
#include <mutex>

namespace PR {
class HitStream;
class IEntity;
class IIntegrator;
class OutputMap;
class RayStream;
class RenderManager;
class RenderTileSession;
class RenderThread;
class RenderTile;
class RenderTileMap;
class Scene;
class PR_LIB RenderContext {
	friend class RenderThread;

	PR_CLASS_NON_COPYABLE(RenderContext);

public:
	RenderContext(uint32 index, uint32 offx, uint32 offy,
				  uint32 width, uint32 height,
				  const std::shared_ptr<Scene>& scene,
				  const RenderManager* manager);
	virtual ~RenderContext();

	inline uint32 index() const { return mIndex; }
	inline uint32 offsetX() const { return mOffsetX; }
	inline uint32 offsetY() const { return mOffsetY; }
	inline uint32 width() const { return mWidth; }
	inline uint32 height() const { return mHeight; }

	uint32 tileCount() const;

	// tcx = tile count x
	// tcy = tile count y
	// tcx and tcy should be able to divide width and height!
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 tcx, uint32 tcy, int32 threads = 0);
	void stop();

	void traceRays(RayStream& rays, HitStream& hits);

	inline bool isStopping() const { return mShouldStop; }
	bool isFinished() const;
	void waitForFinish();

	size_t threads() const { return mThreads.size(); }

	// Pass control
	inline uint32 currentPass() const { return mCurrentPass; }

	// Slow and only copies!
	std::list<RenderTile*> currentTiles() const;

	IIntegrator* integrator() const { return mIntegrator.get(); }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	// Light
	inline const std::vector<IEntity*>& lights() const { return mLights; }

	RenderStatistics statistics() const;
	RenderStatus status() const;

	inline OutputMap* output() const { return mOutputMap.get(); }
	inline const RenderManager* renderManager() const { return mRenderManager; }
	inline Scene* scene() const { return mScene.get(); }

	// Useful settings
	inline uint32 maxRayDepth() const { return mMaxRayDepth; }
	inline uint64 samplesPerPixel() const { return mSamplesPerPixel; }

protected:
	RenderTile* getNextTile();

	void onNextPass();
	void waitForNextPass(); // Never call it from main thread

private:
	void reset();

	const uint32 mIndex;
	const uint32 mOffsetX;
	const uint32 mOffsetY;
	const uint32 mWidth;
	const uint32 mHeight;

	const RenderManager* mRenderManager;
	std::shared_ptr<Scene> mScene;
	std::unique_ptr<OutputMap> mOutputMap;

	std::vector<IEntity*> mLights;

	std::mutex mTileMutex;
	std::unique_ptr<RenderTileMap> mTileMap;
	uint64 mIncrementalCurrentSample;
	std::list<RenderThread*> mThreads;

	const RenderSettings mRenderSettings;

	std::shared_ptr<IIntegrator> mIntegrator;

	std::mutex mPassMutex;
	std::condition_variable mPassCondition;
	uint32 mThreadsWaitingForPass;
	uint32 mCurrentPass;

	bool mShouldStop;

	uint32 mMaxRayDepth;
	uint64 mSamplesPerPixel;
};
} // namespace PR
