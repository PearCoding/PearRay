#include "Renderer.h"
#include "RenderThread.h"
#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "affector/PhotonAffector.h"
#include "affector/LightAffector.h"

#include "integrator/BiDirectIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/DebugIntegrator.h"

#include "Logger.h"
#include "MathUtils.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mMinX(0), mMaxX(1), mMinY(0), mMaxY(1),
		mCamera(cam), mScene(scene),
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
			delete thread;

		mThreads.clear();

		if (mTileMap)
			delete[] mTileMap;

		for (Affector* aff : mAffectors)
			delete aff;

		mAffectors.clear();

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

	uint32 Renderer::renderWidth() const
	{
		return (mMaxX - mMinX) * mWidth;
	}

	void Renderer::setHeight(uint32 h)
	{
		mHeight = h;
	}

	uint32 Renderer::height() const
	{
		return mHeight;
	}

	uint32 Renderer::renderHeight() const
	{
		return (mMaxY - mMinY) * mHeight;
	}

	void Renderer::crop(float xmin, float xmax, float ymin, float ymax)
	{
		mMinX = PM::pm_ClampT(xmin, 0.0f, 1.0f);
		mMaxX = PM::pm_ClampT(xmax, 0.0f, 1.0f);
		mMinY = PM::pm_ClampT(ymin, 0.0f, 1.0f);
		mMaxY = PM::pm_ClampT(ymax, 0.0f, 1.0f);

		if (mMinX > mMaxX)
			std::swap(mMinX, mMaxX);

		if (mMinY > mMaxY)
			std::swap(mMinY, mMaxY);
	}

	void Renderer::start(uint32 tcx, uint32 tcy, uint32 threads, bool clear)
	{
		reset();

		if(clear)
			mResult.clear();

		// Warm up the randomizer
		/*for (uint32 i = 0; i < 800000; ++i)
		{
			mRandom.generate();
		}*/

		/* Setup entities */
		for (Entity* entity : mScene->entities())
			entity->onPreRender();

		if (mRenderSettings.maxPhotons() > 0 &&
			mRenderSettings.maxPhotonGatherRadius() > PM_EPSILON &&
			mRenderSettings.maxPhotonGatherCount() > 0)
			mAffectors.push_back(new PhotonAffector());

		mAffectors.push_back(new LightAffector());

		/* Setup integrators */
		if (mRenderSettings.debugMode() != DM_None)
		{
			mIntegrators.push_back(new DebugIntegrator());
		}
		else
		{
			if (mRenderSettings.maxLightSamples() > 0)
			{
				if (mRenderSettings.isBiDirect())
					mIntegrators.push_back(new BiDirectIntegrator());
				else
					mIntegrators.push_back(new DirectIntegrator(this));
			}
		}

		uint32 threadCount = threads == 0 ? Thread::hardwareThreadCount() : threads;
		for (uint32 i = 0; i < threadCount; ++i)
		{
			RenderThread* thread = new RenderThread(this, i);
			mThreads.push_back(thread);
		}

		for (Affector* affector : mAffectors)
			affector->init(this);

		for (Integrator* integrator : mIntegrators)
			integrator->init(this);

		// Calculate tile sizes, etc.
		mRayCount = 0;
		mPixelsRendered = 0;

		mTileWidth = (uint32)std::ceil(renderWidth() / (float)tcx);
		mTileHeight = (uint32)std::ceil(renderHeight() / (float)tcy);
		mTileMap = new bool[tcx*tcy];
		for (uint32 i = 0; i < tcx*tcy; ++i)
		{
			mTileMap[i] = false;
		}

		PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", threadCount);
		PR_LOGGER.log(L_Info, M_Scene, "Starting threads.");
		for (RenderThread* thread : mThreads)
		{
			thread->start();
		}
	}

	void Renderer::render(RenderContext* context, uint32 x, uint32 y)
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
					default:
					case SM_Jitter:
						sx = x + (xi + mRandom.getFloat()) / (float)mRenderSettings.xSamplerCount() - 0.5f;
						sy = y + (yi + mRandom.getFloat()) / (float)mRenderSettings.ySamplerCount() - 0.5f;
						break;
					}

					Spectrum spec = renderSample(context, sx, sy);

					newSpec += spec;
				}
			}

			const int SampleCount = mRenderSettings.xSamplerCount() * mRenderSettings.ySamplerCount();
			mResult.setPoint(x, y, newSpec / (float)SampleCount);
		}
		else
		{
			Spectrum spec = renderSample(context, x + mRandom.getFloat() - 0.5f,
				y + mRandom.getFloat() - 0.5f);

			mResult.setPoint(x, y,spec);
		}

		mStatisticMutex.lock();
		mPixelsRendered++;
		mStatisticMutex.unlock();
	}

	Spectrum Renderer::renderSample(RenderContext* context, float x, float y)
	{
		Ray ray = mCamera->constructRay(2 * (x + 0.5f) / (float)mWidth - 1.0f,
			2 * (y + 0.5f) / (float)mHeight - 1.0f);// To camera coordinates [-1,1]

		Spectrum spec;
		for (Integrator* integrator : mIntegrators)
		{
			spec += integrator->apply(ray, context);
		}

		return spec;
	}

	size_t Renderer::pixelsRendered() const
	{
		return mPixelsRendered;
	}

	RenderEntity* Renderer::shoot(const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore)
	{
		const uint32 maxDepth = (ray.maxDepth() == 0) ?
			mRenderSettings.maxRayDepth() : PM::pm_MinT<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() < maxDepth)
		{
			RenderEntity* entity = mScene->checkCollision(ray, collisionPoint, ignore);

			const float NdotV = PM::pm_Dot3D(ray.direction(), collisionPoint.normal());
			collisionPoint.setNormal(
				faceforward(NdotV, collisionPoint.normal()));
			collisionPoint.setInside(NdotV > 0);

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

	RenderEntity* Renderer::shootWithApply(Spectrum& appliedSpec, const Ray& ray, FacePoint& collisionPoint, RenderContext* context, RenderEntity* ignore)
	{
		RenderEntity* entity = shoot(ray, collisionPoint, context, ignore);
		if (entity && entity->material())
		{
			for (Affector* affector : mAffectors)
			{
				appliedSpec += affector->apply(ray, entity, collisionPoint, context);
			}
		}
		return entity;
	}

	uint32 Renderer::threads() const
	{
		return (uint32)mThreads.size();
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
		uint32 sliceW = (uint32)std::ceil(renderWidth() / (float)mTileWidth);
		uint32 sliceH = (uint32)std::ceil(renderHeight() / (float)mTileHeight);

		mTileMutex.lock();
		for (uint32 i = 0; i < sliceH; ++i)
		{
			for (uint32 j = 0; j < sliceW; ++j)
			{
				if (!mTileMap[i*sliceW + j])
				{
					sx = mMinX*mWidth + j*mTileWidth;
					sy = mMinY*mHeight + i*mTileHeight;
					ex = PM::pm_MinT<uint32>(mMaxX*mWidth, sx + mTileWidth);
					ey = PM::pm_MinT<uint32>(mMaxY*mHeight, sy + mTileHeight);

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