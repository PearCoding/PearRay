#pragma once

#include "RenderResult.h"
#include "RenderSettings.h"
#include "spectral/Spectrum.h"
#include "Random.h"

#include <list>
#include <mutex>

namespace PR
{
	class Affector;
	class Camera;
	class Entity;
	class FacePoint;
	class Integrator;
	class RenderEntity;
	class Scene;
	class Ray;
	class RenderThread;
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

		void crop(float xmin, float xmax, float ymin, float ymax);
		
		// tcx = tile count x
		// tcy = tile count y
		// tcx and tcy should be able to divide width and height!
		void start(uint32 tcx, uint32 tcy, uint32 threads = 0, bool clear = true);
		void stop();

		bool isFinished();
		void waitForFinish();

		void render(RenderContext* context, uint32 x, uint32 y);

		RenderResult& result();

		inline Random& random()
		{
			return mRandom;
		}

		// Statistics
		size_t rayCount() const;
		size_t pixelsRendered() const;

		// RenderThread things
		RenderEntity* shoot(const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);
		RenderEntity* shootWithApply(Spectrum& appliedSpec, const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore);
		bool getNextTile(uint32& sx, uint32& sy, uint32& ex, uint32& ey);

		uint32 threads() const;

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

		Random mRandom;
		std::list<RenderEntity*> mLights;

		std::mutex mTileMutex;
		uint32 mTileWidth;
		uint32 mTileHeight;
		bool* mTileMap;
		std::list<RenderThread*> mThreads;

		RenderSettings mRenderSettings;

		std::mutex mStatisticMutex;
		size_t mPixelsRendered;
		size_t mRayCount;

		std::list<Affector*> mAffectors;// Per Ray apply
		std::list<Integrator*> mIntegrators;// Per Pixel sample
	};
}