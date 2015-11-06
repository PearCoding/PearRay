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
		mResult(w,h)
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

	void Renderer::render(uint32 threads)
	{
		PR_ASSERT(mThreads.empty());

		mRayCount = 0;

		uint32 threadCount = threads == 0 ? Thread::hardwareThreadCount() : threads;
		uint32 t = PM::pm_MaxT<uint32>(1, threadCount / 2);
		uint32 sliceW = mWidth / t;
		uint32 sliceH = mHeight / t;

		PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", t*t);

		for (uint32 x = 0; x < t; ++x)
		{
			for (uint32 y = 0; y < t; ++y)
			{
				uint32 sx = x*sliceW;
				uint32 sy = y*sliceH;

				RenderThread* thread = new RenderThread(sx, sy, sx + sliceW, sy + sliceH, this);
				mThreads.push_back(thread);
			}
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

	RenderResult& Renderer::result()
	{
		return mResult;
	}

	size_t Renderer::rayCount() const
	{
		return mRayCount;
	}
}