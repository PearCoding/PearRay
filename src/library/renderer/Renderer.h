#pragma once

#include "RenderSettings.h"
#include "spectral/Spectrum.h"

#include <list>
#include <mutex>
#include <atomic>

namespace PR
{
	class Affector;
	class Camera;
	class Entity;
	struct SamplePoint;
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

		void setBackgroundMaterial(Material* m);
		Material* backgroundMaterial() const;
		
		// tcx = tile count x
		// tcy = tile count y
		// tcx and tcy should be able to divide width and height!
		// thread == 0 -> Automatic, thread < 0 -> MaxThreads - k threads, thread > 0 -> k threads
		void start(IDisplayDriver* display, uint32 tcx, uint32 tcy, int32 threads = 0);
		void stop();

		bool isFinished();
		void waitForFinish();

		void render(RenderContext* context, uint32 x, uint32 y, uint32 sample);

		// Statistics
		size_t rayCount() const;
		size_t samplesRendered() const;

		// RenderThread things
		RenderEntity* shoot(const Ray& ray, SamplePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);
		RenderEntity* shootWithApply(Spectrum& appliedSpec, const Ray& ray, SamplePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);

		RenderTile* getNextTile();

		uint32 threads() const;

		// Slow and only copies!
		std::list<RenderTile> currentTiles() const;

		inline GPU* gpu() const
		{
			return mGPU;
		}

		// Settings
		inline void setSettings(const RenderSettings& s)
		{
			mRenderSettings = s;
		}
		
		inline RenderSettings& settings()
		{
			return mRenderSettings;
		}

		// Light
		const std::list<RenderEntity*>& lights() const;

	private:
		void reset();

		Spectrum renderSample(RenderContext* context, float x, float y, float rx, float ry, float t);

		uint32 mWidth;
		uint32 mHeight;

		Camera* mCamera;
		Scene* mScene;
		IDisplayDriver* mResult;

		std::list<RenderEntity*> mLights;
		Material* mBackgroundMaterial;

		std::mutex mTileMutex;
		uint32 mTileWidth;
		uint32 mTileHeight;
		uint32 mTileXCount;
		uint32 mTileYCount;
		RenderTile** mTileMap;
		uint32 mIncrementalCurrentSample;
		std::list<RenderThread*> mThreads;

		RenderSettings mRenderSettings;

		GPU* mGPU;

		// Statistics
		std::atomic<size_t> mRayCount;

		std::list<Affector*> mAffectors;// Per Ray apply
		std::list<Integrator*> mIntegrators;// Per Pixel sample
	};
}