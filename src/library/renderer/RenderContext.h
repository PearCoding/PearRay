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
class OutputBuffer;
class RayStream;
class RenderTileSession;
class RenderThread;
class RenderTile;
class RenderTileMap;
class Scene;
class SpectrumDescriptor;
class PR_LIB RenderContext {
	friend class RenderThread;

	PR_CLASS_NON_COPYABLE(RenderContext);

public:
	RenderContext(uint32 index, uint32 offx, uint32 offy,
				  uint32 width, uint32 height,
				  const std::shared_ptr<IIntegrator>& integrator,
				  const std::shared_ptr<Scene>& scene,
				  const std::shared_ptr<SpectrumDescriptor>& specDesc,
				  const RenderSettings& settings);
	virtual ~RenderContext();

	inline uint32 index() const { return mIndex; }
	inline uint32 offsetX() const { return mOffsetX; }
	inline uint32 offsetY() const { return mOffsetY; }
	inline uint32 width() const { return mWidth; }
	inline uint32 height() const { return mHeight; }

	size_t tileCount() const;
	size_t tileWidth() const;
	size_t tileHeight() const;

	// tcx = tile count x
	// tcy = tile count y
	// tcx and tcy should be able to divide width and height!
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 tcx, uint32 tcy, int32 threads = 0);
	void stop();
	void notifyEnd();

	inline bool isStopping() const { return mShouldStop; }
	bool isFinished() const;
	void waitForFinish();

	size_t threads() const { return mThreads.size(); }

	// Pass control
	inline uint32 currentPass() const { return mCurrentPass; }

	// Slow and only copies!
	std::list<RenderTile*> currentTiles() const;

	std::shared_ptr<IIntegrator> integrator() const { return mIntegrator; }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	// Light
	inline const std::vector<std::shared_ptr<IEntity>>& lights() const { return mLights; }
	inline float emissiveSurfaceArea() const { return mEmissiveSurfaceArea; }

	RenderTileStatistics statistics() const;
	RenderStatus status() const;

	inline std::shared_ptr<OutputBuffer> output() const { return mOutputMap; }
	inline std::shared_ptr<Scene> scene() const { return mScene; }
	inline std::shared_ptr<SpectrumDescriptor> spectrumDescriptor() const { return mSpectrumDescriptor; }

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

	std::shared_ptr<Scene> mScene;
	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;
	std::shared_ptr<OutputBuffer> mOutputMap;

	std::vector<std::shared_ptr<IEntity>> mLights;
	float mEmissiveSurfaceArea;

	std::mutex mTileMutex;
	std::unique_ptr<RenderTileMap> mTileMap;
	uint32 mIncrementalCurrentSample;
	std::list<RenderThread*> mThreads;

	const RenderSettings mRenderSettings;

	std::shared_ptr<IIntegrator> mIntegrator;

	std::mutex mPassMutex;
	std::condition_variable mPassCondition;
	uint32 mThreadsWaitingForPass;
	uint32 mCurrentPass;

	bool mShouldStop;
};
} // namespace PR
