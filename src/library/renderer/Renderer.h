#pragma once

#include "RenderSettings.h"
#include "RenderStatistics.h"
#include "spectral/Spectrum.h"

#include <list>
#include <mutex>
#include <condition_variable>

namespace PR
{
	class Affector;
	class Camera;
	class Entity;
	struct ShaderClosure;
	class GPU;
	class IDisplayDriver;
	class Integrator;
	class Material;
	class Sampler;
	class Scene;
	class Ray;
	class RenderEntity;
	class RenderThread;
	class RenderTile;
	class RenderContext;
	class PR_LIB Renderer
	{
		friend class RenderThread;
		friend class RenderContext;
	public:
		Renderer(uint32 width, uint32 height, Camera* cam, Scene* scene, bool useGPU = true);
		virtual ~Renderer();

		void setWidth(uint32 w);
		uint32 width() const;
		uint32 renderWidth() const; // Depends on crop

		void setHeight(uint32 h);
		uint32 height() const;
		uint32 renderHeight() const; // Depends on crop

		uint32 cropPixelOffsetX() const;
		uint32 cropPixelOffsetY() const;
		
		// tcx = tile count x
		// tcy = tile count y
		// tcx and tcy should be able to divide width and height!
		// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
		void start(IDisplayDriver* display, uint32 tcx, uint32 tcy, int32 threads = 0);
		void stop();

		RenderEntity* shoot(const Ray& ray, ShaderClosure& sc, RenderContext* context, RenderEntity* ignore);
		RenderEntity* shootForDetection(const Ray& ray, RenderContext* context, RenderEntity* ignore);
		RenderEntity* shootWithEmission(Spectrum& appliedSpec, const Ray& ray, ShaderClosure& sc, RenderContext* context, RenderEntity* ignore);

		bool isFinished();
		void waitForFinish();

		uint32 threads() const;

		// Pass control
		inline uint32 currentPass() const
		{
			return mCurrentPass;
		}

		// Slow and only copies!
		std::list<RenderTile> currentTiles() const;

		inline GPU* gpu() const
		{
			return mGPU;
		}

		Integrator* integrator() const
		{
			return mIntegrator;
		}

		// Settings
		inline void setSettings(const RenderSettings& s)
		{
			mRenderSettings = s;
		}
		
		inline RenderSettings& settings() { return mRenderSettings; }
		inline const RenderSettings& settings() const { return mRenderSettings; }

		// Light
		const std::list<RenderEntity*>& lights() const;

		RenderStatistics stats(RenderThread* thread = nullptr) const;
		uint64 maxSamples() const;

		inline Scene* scene() const
		{
			return mScene;
		}
		
	protected:
		// Render Thread specific
		void render(RenderContext* context, uint32 x, uint32 y, uint32 sample, uint32 pass);

		RenderTile* getNextTile();

		void waitForNextPass();// Never call it from main thread

	private:
		void reset();

		Spectrum renderSample(RenderContext* context, float x, float y, float rx, float ry, float t, uint32 pass);

		uint32 mWidth;
		uint32 mHeight;

		Camera* mCamera;
		Scene* mScene;
		IDisplayDriver* mResult;

		std::list<RenderEntity*> mLights;

		std::mutex mTileMutex;
		uint32 mTileWidth;
		uint32 mTileHeight;
		uint32 mTileXCount;
		uint32 mTileYCount;
		RenderTile** mTileMap;
		uint32 mIncrementalCurrentSample;
		std::list<RenderThread*> mThreads;

		RenderSettings mRenderSettings;
		RenderStatistics mGlobalStatistics;

		GPU* mGPU;
		Integrator* mIntegrator;

		std::mutex mPassMutex;
		std::condition_variable mPassCondition;
		uint32 mThreadsWaitingForPass;
		uint32 mCurrentPass;
	};
}