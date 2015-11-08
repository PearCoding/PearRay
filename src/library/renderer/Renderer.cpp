#include "Renderer.h"
#include "RenderThread.h"
#include "scene/Camera.h"
#include "scene/Scene.h"
#include "entity/GeometryEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "Logger.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mCamera(cam), mScene(scene),
		mResult(w, h), mRandom((uint32)time(NULL)),
		mTileWidth(w/8), mTileHeight(h/8), mTileMap(nullptr),
		mMaxRayDepth(3), mMaxRayBounceCount(50), mEnableSubPixels(false)
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
		for (uint32 i = 0; i < 100000; ++i)
		{
			mRandom.generate();
		}

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
		if (mEnableSubPixels)
		{
			float depth1;
			Ray ray1 = renderSubPixels((float)x, (float)y, depth1);

			// Left Sub
			float depth2;
			Ray ray2 = renderSubPixels(x - 0.333f, (float)y, depth2);

			// Right Sub
			float depth3;
			Ray ray3 = renderSubPixels(x + 0.333f, (float)y, depth3);

			// Top Sub
			float depth4;
			Ray ray4 = renderSubPixels((float)x, y - 0.333f, depth4);

			// Bottom Sub
			float depth5;
			Ray ray5 = renderSubPixels((float)x, y + 0.333f, depth5);

			if (depth1 >= 0 || depth2 >= 0 || depth3 >= 0 || depth4 >= 0 || depth5 >= 0)
			{
				float newDepth = (depth1 >= 0 ? depth1*0.333f : 0)
					+ (depth2 >= 0 ? depth2*0.166f : 0)
					+ (depth3 >= 0 ? depth3*0.166f : 0)
					+ (depth4 >= 0 ? depth4*0.166f : 0)
					+ (depth5 >= 0 ? depth5*0.166f : 0);

				Spectrum newSpec;
				if (depth1)
				{
					newSpec += ray1.spectrum()*0.333f;
				}
				if (depth2)
				{
					newSpec += ray2.spectrum()*0.166f;
				}
				if (depth3)
				{
					newSpec += ray3.spectrum()*0.166f;
				}
				if (depth4)
				{
					newSpec += ray4.spectrum()*0.166f;
				}
				if (depth5)
				{
					newSpec += ray5.spectrum()*0.166f;
				}

				mResult.setDepth(x, y, newDepth);
				mResult.setPoint(x, y, newSpec);
			}
		}
		else
		{
			float depth;
			Ray ray = renderSubPixels((float)x, (float)y, depth);
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

	Ray Renderer::renderSubPixels(float x, float y, float& depth)
	{
		float sx = mCamera->width() * x / (float)mWidth - mCamera->width() / 2.0f;
		float sy = mCamera->height() * y / (float)mHeight - mCamera->height() / 2.0f;

		Ray ray = mCamera->constructRay(sx, sy);

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

	GeometryEntity* Renderer::shoot(Ray& ray, FacePoint& collisionPoint)
	{
		GeometryEntity* entity = mScene->checkCollision(ray, collisionPoint);

		if (entity)
		{
			//ray.setSpectrum(mIdentitySpectrum);
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

	void Renderer::enableSubPixels(bool b)
	{
		mEnableSubPixels = b;
	}

	bool Renderer::isSubPixelsEnalbed() const
	{
		return mEnableSubPixels;
	}
}