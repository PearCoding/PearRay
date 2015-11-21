#include "Renderer.h"
#include "RenderThread.h"
#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/GeometryEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "Logger.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mCamera(cam), mScene(scene),
		mResult(w, h), mRandom((uint64)time(NULL)),
		mTileWidth(w/8), mTileHeight(h/8), mTileMap(nullptr),
		mMaxRayDepth(3), mMaxRayBounceCount(50),
		mEnableSampling(false), mSamplePerRayCount(8)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			mIdentitySpectrum.setValue(i, 1);
		}
	}

	Renderer::~Renderer()
	{
		reset();
	}

	void Renderer::reset()
	{
		for (RenderThread* thread : mThreads)
		{
			delete thread;
		}

		mThreads.clear();

		if (mTileMap)
		{
			delete[] mTileMap;
		}
	}

	void Renderer::setWidth(uint32 w)
	{
		mWidth = w;
	}

	uint32 Renderer::width() const
	{
		return mWidth;
	}

	void Renderer::setHeight(uint32 h)
	{
		mHeight = h;
	}

	uint32 Renderer::height() const
	{
		return mHeight;
	}

	void Renderer::start(uint32 tcx, uint32 tcy, uint32 threads)
	{
		reset();
		mResult.clear();

		// Warm up the randomizer
		/*for (uint32 i = 0; i < 800000; ++i)
		{
			mRandom.generate();
		}*/

		mRayCount = 0;
		mPixelsRendered = 0;

		mTileWidth = (uint32)std::ceil(mWidth / (float)tcx);
		mTileHeight = (uint32)std::ceil(mHeight / (float)tcy);
		mTileMap = new bool[tcx*tcy];
		for (uint32 i = 0; i < tcx*tcy; ++i)
		{
			mTileMap[i] = false;
		}

		uint32 threadCount = threads == 0 ? Thread::hardwareThreadCount() : threads;

		PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", threadCount);

		for (uint32 i = 0; i < threadCount; ++i)
		{
			RenderThread* thread = new RenderThread(this);
			mThreads.push_back(thread);
		}

		PR_LOGGER.log(L_Info, M_Scene, "Starting threads.");
		for (RenderThread* thread : mThreads)
		{
			thread->start();
		}
	}

	void Renderer::render(uint32 x, uint32 y)
	{
		if (mEnableSampling)
		{
			uint32 successfulSamples = 0;
			float newDepth = 0;
			Spectrum newSpec;

			for (uint32 i = 0; i < mSamplePerRayCount; ++i)
			{
				float depth;
				Ray ray = renderSample((float)x + (mRandom.getFloat() - 0.5f),
					(float)y + (mRandom.getFloat() - 0.5f),
					depth);

				if (depth >= 0)
				{
					newDepth += depth;
					newSpec += ray.spectrum();
					++successfulSamples;
				}
			}

			if (successfulSamples > 0)
			{
				mResult.setDepth(x, y, newDepth / successfulSamples);
				mResult.setPoint(x, y, newSpec / (float)successfulSamples);
			}
		}
		else
		{
			float depth;
			Ray ray = renderSample((float)x, (float)y, depth);
			if (depth >= 0)
			{
				mResult.setDepth(x, y, depth);
				mResult.setPoint(x, y, ray.spectrum());
			}
		}

		mStatisticMutex.lock();
		mPixelsRendered++;
		mStatisticMutex.unlock();
	}

	Ray Renderer::renderSample(float x, float y, float& depth)
	{
		Ray ray = mCamera->constructRay(2 * x / (float)mWidth - 1,
			2 * y / (float)mHeight - 1);

		FacePoint collisionPoint;
		GeometryEntity* entity = shoot(ray, collisionPoint);

		if (entity)
		{
			depth = PM::pm_Magnitude3D(PM::pm_Subtract(collisionPoint.vertex(), ray.startPosition()));
		}
		else
		{
			depth = -1;
		}

		return ray;
	}

	size_t Renderer::pixelsRendered() const
	{
		return mPixelsRendered;
	}

	GeometryEntity* Renderer::shoot(Ray& ray, FacePoint& collisionPoint, Entity* ignore)
	{
		GeometryEntity* entity = mScene->checkCollision(ray, collisionPoint, ignore);

		if (entity)
		{
			entity->apply(ray, collisionPoint, this);
		}

		mStatisticMutex.lock();
		mRayCount++;
		mStatisticMutex.unlock();

		return entity;
	}

	bool Renderer::isFinished()
	{
		for (RenderThread* thread : mThreads)
		{
			if (thread->state() != Thread::S_Stopped)
			{
				return false;
			}
		}

		return true;
	}

	void Renderer::waitForFinish()
	{
		while (!isFinished())
		{
			std::this_thread::yield();
		}
	}

	void Renderer::stop()
	{
		for (RenderThread* thread : mThreads)
		{
			thread->stop();
		}
	}

	bool Renderer::getNextTile(uint32& sx, uint32& sy, uint32& ex, uint32& ey)
	{
		uint32 sliceW = (uint32)std::ceil(mWidth / (float)mTileWidth);
		uint32 sliceH = (uint32)std::ceil(mHeight / (float)mTileHeight);

		mTileMutex.lock();
		for (uint32 i = 0; i < sliceH; ++i)
		{
			for (uint32 j = 0; j < sliceW; ++j)
			{
				if (!mTileMap[i*sliceW + j])
				{
					sx = j*mTileWidth;
					sy = i*mTileHeight;
					ex = PM::pm_MinT<uint32>(mWidth, sx + mTileWidth);
					ey = PM::pm_MinT<uint32>(mHeight, sy + mTileHeight);

					mTileMap[i*sliceW + j] = true;
					mTileMutex.unlock();

					return true;
				}
			}
		}
		mTileMutex.unlock();
		return false;
	}

	RenderResult& Renderer::result()
	{
		return mResult;
	}

	size_t Renderer::rayCount() const
	{
		return mRayCount;
	}

	void Renderer::setMaxRayDepth(uint32 i)
	{
		mMaxRayDepth = i;
	}

	uint32 Renderer::maxRayDepth() const
	{
		return mMaxRayDepth;
	}

	void Renderer::setMaxRayBounceCount(uint32 i)
	{
		mMaxRayBounceCount = i;
	}

	uint32 Renderer::maxRayBounceCount() const
	{
		return mMaxRayBounceCount;
	}

	void Renderer::enableSampling(bool b)
	{
		mEnableSampling = b;
	}

	bool Renderer::isSamplingEnalbed() const
	{
		return mEnableSampling;
	}
}