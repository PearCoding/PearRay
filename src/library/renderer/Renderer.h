#pragma once

#include "RenderResult.h"
#include "spectral/Spectrum.h"

#include <list>
#include <mutex>

namespace PR
{
	class Camera;
	class FacePoint;
	class GeometryEntity;
	class Scene;
	class Ray;
	class RenderThread;
	class PR_LIB Renderer
	{
	public:
		Renderer(uint32 width, uint32 height, Camera* cam, Scene* scene);
		virtual ~Renderer();

		void setWidth(uint32 w);
		uint32 width() const;

		void setHeight(uint32 h);
		uint32 height() const;
		
		// tcx = tile count x
		// tcy = tile count y
		// tcx and tcy should be able to divide width and height!
		void start(uint32 tcx, uint32 tcy, uint32 threads = 0);
		void stop();

		bool isFinished();
		void waitForFinish();

		void render(uint32 x, uint32 y);

		RenderResult& result();

		// Statistics
		size_t rayCount() const;
		size_t pixelsRendered() const;

		// RenderThread things
		GeometryEntity* shoot(Ray& ray, FacePoint& collisionPoint);
		bool getNextTile(uint32& sx, uint32& sy, uint32& ex, uint32& ey);

		// Settings
		void setMaxRayDepth(uint32 i);
		uint32 maxRayDepth() const;

		void setMaxRayBounceCount(uint32 i);
		uint32 maxRayBounceCount() const;

		void enableSubPixels(bool b);
		bool isSubPixelsEnalbed() const;

	private:
		void reset();

		Ray renderSubPixels(float x, float y, float& depth);

		uint32 mWidth;
		uint32 mHeight;

		Camera* mCamera;
		Scene* mScene;
		RenderResult mResult;

		std::mutex mTileMutex;
		uint32 mTileWidth;
		uint32 mTileHeight;
		bool* mTileMap;
		std::list<RenderThread*> mThreads;

		Spectrum mIdentitySpectrum;

		std::mutex mStatisticMutex;
		size_t mPixelsRendered;
		size_t mRayCount;

		// Settings
		uint32 mMaxRayDepth;
		uint32 mMaxRayBounceCount;
		bool mEnableSubPixels;
	};
}