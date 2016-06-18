#pragma once

#include "RenderResult.h"
#include "RenderSettings.h"
#include "spectral/Spectrum.h"

#include <list>
#include <mutex>

namespace PR
{
	class Affector;
	class Camera;
	class Entity;
	class FacePoint;
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
		Renderer(uint32 width, uint32 height, Camera* cam, Scene* scene);
		virtual ~Renderer();

		void setWidth(uint32 w);
		uint32 width() const;
		uint32 renderWidth() const; // Depends on crop

		void setHeight(uint32 h);
		uint32 height() const;
		uint32 renderHeight() const; // Depends on crop

		void setBackgroundMaterial(Material* m);
		Material* backgroundMaterial() const;

		void crop(float xmin, float xmax, float ymin, float ymax);
		
		// tcx = tile count x
		// tcy = tile count y
		// tcx and tcy should be able to divide width and height!
		void start(uint32 tcx, uint32 tcy, uint32 threads = 0, bool clear = true);
		void stop();

		bool isFinished();
		void waitForFinish();

		void render(RenderContext* context, uint32 x, uint32 y, uint32 sample);

		RenderResult& result();

		// Statistics
		size_t rayCount() const;
		size_t samplesRendered() const;

		// RenderThread things
		RenderEntity* shoot(const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);
		RenderEntity* shootWithApply(Spectrum& appliedSpec, const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);

		RenderTile* getNextTile();

		uint32 threads() const;

		// Slow and only copies!
		std::list<RenderTile> currentTiles() const;

		// Settings
		inline RenderSettings& settings()
		{
			return mRenderSettings;
		}

		// Light
		const std::list<RenderEntity*>& lights() const;

	private:
		void reset();

		Spectrum renderSample(RenderContext* context, float x, float y);

		uint32 mWidth;
		uint32 mHeight;

		float mMinX;
		float mMaxX;
		float mMinY;
		float mMaxY;

		Camera* mCamera;
		Scene* mScene;
		RenderResult mResult;

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

		std::mutex mStatisticMutex;
		size_t mRayCount;

		std::list<Affector*> mAffectors;// Per Ray apply
		std::list<Integrator*> mIntegrators;// Per Pixel sample
	};
}