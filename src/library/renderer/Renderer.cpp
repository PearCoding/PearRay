#include "Renderer.h"
#include "RenderThread.h"
#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "integrator/DirectIntegrator.h"
#include "integrator/PhotonIntegrator.h"
#include "integrator/LightIntegrator.h"

#include "Logger.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mCamera(cam), mScene(scene),
		mResult(w, h), mRandom((uint64)time(NULL)),
		mTileWidth(w/8), mTileHeight(h/8), mTileMap(nullptr)
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

		for (Integrator* integrator : mIntegrators)
			delete integrator;

		mIntegrators.clear();
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

		/* Setup entities */
		for (Entity* entity : mScene->entities())
		{
			entity->onPreRender();
		}

		/* Setup integrators */
		if (mRenderSettings.maxLightSamples() > 0)
			mIntegrators.push_back(new DirectIntegrator(this, mRenderSettings.maxLightSamples()));
		
		//if (mRenderSettings.maxPhotons() > 0)
			//mIntegrators.push_back(new PhotonIntegrator(this));

		mIntegrators.push_back(new LightIntegrator());

		for (Integrator* integrator : mIntegrators)
			integrator->init(this);

		// Calculate tile sizes, etc.
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
		if (mRenderSettings.samplerMode() != SM_None)
		{
			float newDepth = 0;
			Spectrum newSpec;

			for (uint32 yi = 0; yi < mRenderSettings.ySamplerCount(); ++yi)
			{
				for (uint32 xi = 0; xi < mRenderSettings.xSamplerCount(); ++xi)
				{
					float sx;
					float sy;

					switch (mRenderSettings.samplerMode())
					{
					case SM_Random:
						sx = x + mRandom.getFloat() - 0.5f;
						sy = y + mRandom.getFloat() - 0.5f;
						break;
					case SM_Uniform:
						sx = x + xi / (float)mRenderSettings.xSamplerCount() - 0.5f;
						sy = y + yi / (float)mRenderSettings.ySamplerCount() - 0.5f;
						break;
					case SM_Jitter:
						sx = x + (xi + mRandom.getFloat()) / (float)mRenderSettings.xSamplerCount() - 0.5f;
						sy = y + (yi + mRandom.getFloat()) / (float)mRenderSettings.ySamplerCount() - 0.5f;
						break;
					}

					float depth;
					Ray ray = renderSample(sx, sy, depth);

					if (depth >= 0)
					{
						newDepth += depth;
						newSpec += ray.spectrum();
					}

				}
			}

			const int SampleCount = mRenderSettings.xSamplerCount() * mRenderSettings.ySamplerCount();
			mResult.setDepth(x, y, newDepth / SampleCount);
			mResult.setPoint(x, y, newSpec / SampleCount);
		}
		else
		{
			float depth;// Random sampling
			Ray ray = renderSample(x + mRandom.getFloat() - 0.5f,
				y + mRandom.getFloat() - 0.5f,
				depth);

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
		Ray ray = mCamera->constructRay(2 * (x + 0.5f) / (float)mWidth - 1.0f,
			2 * (y + 0.5f) / (float)mHeight - 1.0f);// To camera coordinates [-1,1]

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
		const uint32 maxDepth = ray.maxDepth() == 0 ?
			mRenderSettings.maxRayDepth() : PM::pm_MinT<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() < maxDepth)
		{
			RenderEntity* entity = mScene->checkCollision(ray, collisionPoint, ignore);

			if (entity && entity->material())
			{
				Spectrum spec;
				for (Integrator* integrator : mIntegrators)
				{
					spec += integrator->apply(ray, entity, collisionPoint, this);
				}

				ray.setSpectrum(spec);
			}

			mStatisticMutex.lock();
			mRayCount++;
			mStatisticMutex.unlock();

			return entity;
		}
		else
		{
			return nullptr;
		}
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

	const std::list<RenderEntity*>& Renderer::lights() const
	{
		return mLights;
	}
}