#pragma once

#include "RenderResult.h"
#include "spectral/Spectrum.h"
#include "Random.h"

#include <list>
#include <mutex>

namespace PR
{
	enum SamplerMode
	{
		SM_Random,
		SM_Uniform,
		SM_Jitter
	};

	class Camera;
	class Entity;
	class FacePoint;
	class RenderEntity;
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

		inline Random& random()
		{
			return mRandom;
		}

		// Statistics
		size_t rayCount() const;
		size_t pixelsRendered() const;

		// RenderThread things
		RenderEntity* shoot(Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore = nullptr);
		bool getNextTile(uint32& sx, uint32& sy, uint32& ex, uint32& ey);

		// Settings
		void setMaxRayDepth(uint32 i);
		uint32 maxRayDepth() const;

		void setMaxDirectRayCount(uint32 i);
		uint32 maxDirectRayCount() const;

		void setMaxIndirectRayCount(uint32 i);
		uint32 maxIndirectRayCount() const;

		void enableSampling(bool b);
		bool isSamplingEnalbed() const;

		void setSamplePerRayCount(uint32 i)
		{
			mSamplePerRayCount = i;
		}

		inline uint32 samplePerRayCount() const
		{
			return mSamplePerRayCount;
		}

		void setSamplerMode(SamplerMode mode)
		{
			mSamplerMode = mode;
		}

		inline SamplerMode samplerMode() const
		{
			return mSamplerMode;
		}

		// Light
		const std::list<RenderEntity*>& lights() const;

	private:
		void reset();

		Ray renderSample(float x, float y, float& depth);

		uint32 mWidth;
		uint32 mHeight;

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

		Spectrum mIdentitySpectrum;

		std::mutex mStatisticMutex;
		size_t mPixelsRendered;
		size_t mRayCount;

		// Settings
		uint32 mMaxRayDepth;
		uint32 mMaxDirectRayCount;
		uint32 mMaxIndirectRayCount;
		bool mEnableSampling;
		uint32 mSamplePerRayCount;
		SamplerMode mSamplerMode;
	};
}