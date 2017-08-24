#pragma once

#include "RenderSettings.h"
#include "RenderStatistics.h"
#include "RenderStatus.h"
#include "spectral/Spectrum.h"

#include <Eigen/Dense>
#include <condition_variable>
#include <list>
#include <mutex>

namespace PR {
class Camera;
class Entity;
class GPU;
class Integrator;
class OutputMap;
class Ray;
class RenderEntity;
class RenderThread;
class RenderTile;
class RenderThreadContext;
class Scene;
struct ShaderClosure;
class PR_LIB RenderContext {
	friend class RenderThread;

	PR_CLASS_NON_COPYABLE(RenderContext);

public:
	RenderContext(uint32 index, uint32 offx, uint32 offy, uint32 width, uint32 height, uint32 fwidth, uint32 fheight,
				  const Scene& scene, const std::string& workingDir, GPU* gpu, const RenderSettings& settings);
	virtual ~RenderContext();

	inline uint32 index() const { return mIndex; }
	inline uint32 offsetX() const { return mOffsetX; }
	inline uint32 offsetY() const { return mOffsetY; }
	inline uint32 width() const { return mWidth; }
	inline uint32 height() const { return mHeight; }
	inline uint32 fullWidth() const { return mFullWidth; }
	inline uint32 fullHeight() const { return mFullHeight; }

	inline uint32 tileCount() const { return mTileXCount * mTileYCount; }

	// tcx = tile count x
	// tcy = tile count y
	// tcx and tcy should be able to divide width and height!
	// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
	void start(uint32 tcx, uint32 tcy, int32 threads = 0);
	void stop();

	RenderEntity* shoot(const Ray& ray, ShaderClosure& sc, RenderTile* tile);
	bool shootForDetection(const Ray& ray, RenderTile* tile);
	RenderEntity* shootWithEmission(Spectrum& appliedSpec, const Ray& ray, ShaderClosure& sc, RenderTile* tile);
	void render(RenderTile* tile, const Eigen::Vector2i& pixel,
				uint32 sample, uint32 pass);

	inline bool isStopping() const { return mShouldStop; }
	bool isFinished() const;
	void waitForFinish();

	size_t threads() const { return mThreads.size(); }

	// Pass control
	inline uint32 currentPass() const { return mCurrentPass; }

	// Slow and only copies!
	std::list<RenderTile> currentTiles() const;

	Integrator* integrator() const { return mIntegrator; }

	// Settings
	inline const RenderSettings& settings() const { return mRenderSettings; }

	// Light
	inline const std::list<RenderEntity*>& lights() const { return mLights; }

	RenderStatistics statistics() const;
	RenderStatus status() const;

	inline const Scene& scene() const { return mScene; }
	inline std::string workingDir() const { return mWorkingDir; }
	inline OutputMap* output() const { return mOutputMap; }
	inline GPU* gpu() const { return mGPU; }

protected:
	RenderTile* getNextTile();

	void onNextPass();
	void waitForNextPass(); // Never call it from main thread

private:
	void reset();

	void renderIncremental(RenderTile* tile, const Eigen::Vector2i& pixel,
						   uint32 sample, uint32 pass);
	Spectrum renderSample(RenderTile* tile,
						  float x, float y, float rx, float ry, float t, uint8 wavelength,
						  uint32 pass, ShaderClosure& sc);

	const uint32 mIndex;
	const uint32 mOffsetX;
	const uint32 mOffsetY;
	const uint32 mWidth;
	const uint32 mHeight;
	const uint32 mFullWidth;
	const uint32 mFullHeight;
	const std::string mWorkingDir;

	const std::shared_ptr<Camera> mCamera;
	const Scene& mScene;
	OutputMap* mOutputMap;

	std::list<RenderEntity*> mLights;

	std::mutex mTileMutex;
	uint32 mTileWidth;
	uint32 mTileHeight;
	uint32 mTileXCount;
	uint32 mTileYCount;
	RenderTile** mTileMap;
	uint32 mIncrementalCurrentSample;
	std::list<RenderThread*> mThreads;

	const RenderSettings mRenderSettings;

	GPU* const mGPU;
	Integrator* mIntegrator;

	std::mutex mPassMutex;
	std::condition_variable mPassCondition;
	uint32 mThreadsWaitingForPass;
	uint32 mCurrentPass;

	bool mShouldStop;
};
}
