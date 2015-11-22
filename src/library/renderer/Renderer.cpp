#include "Renderer.h"
#include "RenderThread.h"
#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "Logger.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mCamera(cam), mScene(scene),
		mResult(w, h), mRandom((uint64)time(NULL)),
		mTileWidth(w/8), mTileHeight(h/8), mTileMap(nullptr),
		mMaxRayDepth(3), mMaxDirectRayCount(10), mMaxIndirectRayCount(50),
		mEnableSampling(false), mSamplePerRayCount(8), mSamplerMode(SM_Jitter)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		for (RenderEntity* e : scene->renderEntities())
		{
			if (e->isLight())
			{
				mLights.push_back(e);
			}
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

			const uint32 xSamples = mSamplePerRayCount / 2;
			const uint32 ySamples = mSamplePerRayCount - xSamples;
			const float xSampleMid = 1 / (2.0f * xSamples);
			const float ySampleMid = 1 / (2.0f * ySamples);

			for (uint32 i = 0; i < mSamplePerRayCount; ++i)
			{
				float sx;
				float sy;

				if (mSamplerMode == SM_Random)
				{
					sx = x + mRandom.getFloat() - 0.5f;
					sy = y + mRandom.getFloat() - 0.5f;
				}
				else if (mSamplerMode == SM_Uniform)
				{
					int tx = i % xSamples;
					int ty = i / xSamples;

					sx = x + tx / (float)xSamples - 0.5f;
					sy = y + ty / (float)ySamples - 0.5f;
				}
				else //SM_Jitter
				{
					int tx = i % xSamples;
					int ty = i / xSamples;

					sx = x + (tx + mRandom.getFloat()) / (float)xSamples - 0.5f;
					sy = y + (ty + mRandom.getFloat()) / (float)ySamples - 0.5f;
				}

				float depth;
				Ray ray = renderSample(sx, sy, depth);

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
		RenderEntity* entity = shoot(ray, collisionPoint);

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

	RenderEntity* Renderer::shoot(Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore)
	{
		RenderEntity* entity = mScene->checkCollision(ray, collisionPoint, ignore);

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

	void Renderer::setMaxDirectRayCount(uint32 i)
	{
		mMaxDirectRayCount = i;
	}

	uint32 Renderer::maxDirectRayCount() const
	{
		return mMaxDirectRayCount;
	}

	void Renderer::setMaxIndirectRayCount(uint32 i)
	{
		mMaxIndirectRayCount = i;
	}

	uint32 Renderer::maxIndirectRayCount() const
	{
		return mMaxIndirectRayCount;
	}

	void Renderer::enableSampling(bool b)
	{
		mEnableSampling = b;
	}

	bool Renderer::isSamplingEnalbed() const
	{
		return mEnableSampling;
	}

	const std::list<RenderEntity*>& Renderer::lights() const
	{
		return mLights;
	}
}