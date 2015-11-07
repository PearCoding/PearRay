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
		mResult(w,h), mTileWidth(w/8), mTileHeight(h/8), mTileMap(nullptr),
		mMaxRayDepth(3), mMaxRayBounceCount(50)
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
		for (RenderThread* thread : mThreads)
		{
			delete thread;
		}

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

	void Renderer::render(uint32 tcx, uint32 tcy, uint32 threads)
	{
		PR_ASSERT(mThreads.empty());

		mRayCount = 0;
		mPixelsRendered = 0;

		mTileWidth = width()/tcx;
		mTileHeight = height()/tcy;
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
		float sx = mCamera->width() * x / (float)mWidth - mCamera->width() / 2.0f;
		float sy = mCamera->height() * y / (float)mHeight - mCamera->height() / 2.0f;

		PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Set(sx, sy, mCamera->lensDistance()));

		Ray ray(PM::pm_Multiply(mCamera->matrix(), PM::pm_Set(sx, sy, 0)),
			PM::pm_Multiply(PM::pm_Rotation(mCamera->rotation()), dir));

		FacePoint collisionPoint;
		GeometryEntity* entity = shoot(ray, collisionPoint);

		if (entity)
		{
			float newDepth = PM::pm_Magnitude3D(PM::pm_Subtract(collisionPoint.vertex(), ray.startPosition()));
			mResult.setDepth(x, y, newDepth);
			mResult.setPoint(x, y, ray.spectrum());
		}

		mStatisticMutex.lock();
		mPixelsRendered++;
		mStatisticMutex.unlock();
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
		uint32 sliceW = mWidth / mTileWidth;
		uint32 sliceH = mHeight / mTileHeight;

		mTileMutex.lock();
		for (uint32 i = 0; i < sliceH; ++i)
		{
			for (uint32 j = 0; j < sliceW; ++j)
			{
				if (!mTileMap[i*sliceW + j])
				{
					sx = j*mTileWidth;
					sy = i*mTileHeight;
					ex = sx + mTileWidth;
					ey = sy + mTileHeight;

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
}