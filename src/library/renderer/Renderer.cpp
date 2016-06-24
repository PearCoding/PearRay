#include "Renderer.h"
#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"

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

#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/UniformSampler.h"

#include "Logger.h"
#include "math/Reflection.h"

#include "material/Material.h"

#ifndef PR_NO_GPU
# include "gpu/GPU.h"
#endif

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene) :
		mWidth(w), mHeight(h), mMinX(0), mMaxX(1), mMinY(0), mMaxY(1),
		mCamera(cam), mScene(scene),
		mResult(w, h), mBackgroundMaterial(nullptr),
		mTileWidth(w/8), mTileHeight(h/8), mTileXCount(8), mTileYCount(8), mIncrementalCurrentSample(0), mTileMap(nullptr),
		mGPU(nullptr)
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

		// Setup GPU
#ifndef PR_NO_GPU
		mGPU = new GPU();
		if (!mGPU->init("", "../src/library/cl"))
		{
			delete mGPU;
			mGPU = nullptr;
		}
#endif
	}

	Renderer::~Renderer()
	{
		reset();

#ifndef PR_NO_GPU
		if (mGPU)
		{
			delete mGPU;
		}
#endif
	}

	void Renderer::reset()
	{
		for (RenderThread* thread : mThreads)
			delete thread;

		mThreads.clear();

		if (mTileMap)
		{
			for (uint32 i = 0; i < mTileXCount*mTileYCount; ++i)
			{
				delete mTileMap[i];
			}

			delete[] mTileMap;
			mTileMap = nullptr;
		}

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
		return (uint32)std::ceil((mMaxX - mMinX) * mWidth);
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
		return (uint32)std::ceil((mMaxY - mMinY) * mHeight);
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

	void Renderer::setBackgroundMaterial(Material* m)
	{
		mBackgroundMaterial = m;
	}

	Material* Renderer::backgroundMaterial() const
	{
		return mBackgroundMaterial;
	}

	void Renderer::start(uint32 tcx, uint32 tcy, int32 threads, bool clear)
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

		/* Setup threads */
		uint32 threadCount = Thread::hardwareThreadCount();
		if (threads < 0)
		{
			threadCount = PM::pm_MaxT(1, (int32)threadCount + threads);
		}
		else if(threads > 0)
		{
			threadCount = threads;
		}

		for (uint32 i = 0; i < threadCount; ++i)
		{
			RenderThread* thread = new RenderThread(this, i);
			mThreads.push_back(thread);

			RenderContext& context = thread->context();

			/* Setup samplers */
			switch (mRenderSettings.pixelSampler())
			{
			case SM_Random:
				context.setPixelSampler(new RandomSampler(context.random()));
				break;
			case SM_Uniform:
				context.setPixelSampler(new UniformSampler(context.random(), mRenderSettings.maxPixelSampleCount()));
				break;
			case SM_Jitter:
				context.setPixelSampler(new StratifiedSampler(context.random(), mRenderSettings.maxPixelSampleCount()));
				break;
			default:
			case SM_MultiJitter:
				context.setPixelSampler(new MultiJitteredSampler(context.random(), mRenderSettings.maxPixelSampleCount()));
				break;
			}
		}

		for (Affector* affector : mAffectors)
			affector->init(this);

		for (Integrator* integrator : mIntegrators)
			integrator->init(this);

		// Calculate tile sizes, etc.
		mRayCount = ATOMIC_VAR_INIT(0);

		mTileXCount = tcx;
		mTileYCount = tcy;
		mTileWidth = (uint32)std::ceil(renderWidth() / (float)tcx);
		mTileHeight = (uint32)std::ceil(renderHeight() / (float)tcy);
		mTileMap = new RenderTile*[tcx*tcy];
		for (uint32 i = 0; i < tcy; ++i)
		{
			for (uint32 j = 0; j < tcx; ++j)
			{
				uint32 sx = (uint32)std::ceil(mMinX*mWidth + j*mTileWidth);
				uint32 sy = (uint32)std::ceil(mMinY*mHeight + i*mTileHeight);
				mTileMap[i*tcx + j] = new RenderTile(
					sx,
					sy,
					PM::pm_MinT((uint32)std::ceil(mMaxX*mWidth), sx + mTileWidth),
					PM::pm_MinT((uint32)std::ceil(mMaxY*mHeight), sy + mTileHeight));
			}
		}
		mIncrementalCurrentSample = 0;

		PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", threadCount);
		PR_LOGGER.log(L_Info, M_Scene, "Starting threads.");
		for (RenderThread* thread : mThreads)
		{
			thread->start();
		}
	}

	void Renderer::render(RenderContext* context, uint32 x, uint32 y, uint32 sample)
	{
		if (mRenderSettings.isIncremental())// Only one sample a time!
		{
			Spectrum oldSpec;
			if (sample > 0)
			{
				oldSpec = mResult.point(x, y);
			}

			auto s = context->pixelSampler()->generate2D(sample);

			float rx = context->random().getFloat();// Random sampling
			float ry = context->random().getFloat();
			
			Spectrum newSpec = renderSample(context, x + PM::pm_GetX(s) - 0.5f, y + PM::pm_GetY(s) - 0.5f,
				rx, ry,//TODO
				0.0f);//TODO

			if (sample > 0)
			{
				mResult.setPoint(x, y,
					oldSpec * (sample / (sample + 1.0f)) + newSpec * (1.0f / (sample + 1.0f)));
			}
			else
			{
				mResult.setPoint(x, y, newSpec);
			}
		}
		else// Everything
		{
			const uint32 SampleCount = mRenderSettings.maxPixelSampleCount();
			Spectrum newSpec;

			for (uint32 i = sample; i < SampleCount; ++i)
			{
				auto s = context->pixelSampler()->generate2D(i);

				float rx = context->random().getFloat();// Random sampling
				float ry = context->random().getFloat();

				Spectrum spec = renderSample(context, x + PM::pm_GetX(s) - 0.5f, y + PM::pm_GetY(s) - 0.5f,
					rx, ry,//TODO
					0.0f);
				newSpec += spec;
			}

			mResult.setPoint(x, y, newSpec / (float)SampleCount);
		}
	}

	Spectrum Renderer::renderSample(RenderContext* context, float x, float y, float rx, float ry, float t)
	{
		Ray ray = mCamera->constructRay(2 * (x + 0.5f) / (float)mWidth - 1.0f,
			2 * (y + 0.5f) / (float)mHeight - 1.0f,// To camera coordinates [-1,1]
			rx, ry, t);

		Spectrum spec;
		for (Integrator* integrator : mIntegrators)
		{
			spec += integrator->apply(ray, context);
		}

		return spec;
	}

	size_t Renderer::samplesRendered() const
	{
		size_t s = 0;
		for(RenderThread* thread : mThreads)
		{
			s += thread->samplesRendered();
		}

		return s;
	}
	
	std::list<RenderTile> Renderer::currentTiles() const
	{
		std::list<RenderTile> list;
		for (RenderThread* thread : mThreads)
		{
			RenderTile* tile = thread->currentTile();
			if (tile)
				list.push_back(*tile);
		}
		return list;
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

			mRayCount++;

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
		if (entity)
		{
			for (Affector* affector : mAffectors)
			{
				appliedSpec += affector->apply(ray, entity, collisionPoint, context);
			}
		}
		else if (mBackgroundMaterial)
		{
			FacePoint point;
			point.setMaterial(mBackgroundMaterial);
			point.setInside(true);
			point.setNormal(PM::pm_Negate(ray.direction()));
			point.setVertex(ray.direction());//Radius?

			float u = 0.5f + std::atan2(PM::pm_GetZ(ray.direction()), PM::pm_GetX(ray.direction())) * PM_INV_PI_F * 0.5f;
			float v = 0.5f - std::asin(-PM::pm_GetY(ray.direction())) * PM_INV_PI_F;
			point.setUV(PM::pm_Set(u, v));

			PR_DEBUG_ASSERT(u >= 0 && u <= 1);
			PR_DEBUG_ASSERT(v >= 0 && v <= 1);

			appliedSpec += mBackgroundMaterial->applyEmission(point, ray.direction()) /** (4*PM_INV_PI_F)*/;
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

	RenderTile* Renderer::getNextTile()
	{
		mTileMutex.lock();
		if (mRenderSettings.isIncremental())
		{
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for (uint32 j = 0; j < mTileXCount; ++j)
				{
					if ( mTileMap[i*mTileXCount + j]->samplesRendered() <= mIncrementalCurrentSample &&
						mTileMap[i*mTileXCount + j]->samplesRendered() < mRenderSettings.maxPixelSampleCount() &&
						!mTileMap[i*mTileXCount + j]->isWorking())
					{
						mTileMap[i*mTileXCount + j]->setWorking(true);
						mTileMutex.unlock();

						return mTileMap[i*mTileXCount + j];
					}
				}
			}

			if (mIncrementalCurrentSample < mRenderSettings.maxPixelSampleCount())// Try again
			{
				mIncrementalCurrentSample++;
				mTileMutex.unlock();
				return getNextTile();
			}
		}
		else
		{
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for (uint32 j = 0; j < mTileXCount; ++j)
				{
					if (mTileMap[i*mTileXCount + j]->samplesRendered() == 0 &&
						mTileMap[i*mTileXCount + j]->samplesRendered() < mRenderSettings.maxPixelSampleCount() &&
						!mTileMap[i*mTileXCount + j]->isWorking())
					{
						mTileMap[i*mTileXCount + j]->setWorking(true);
						mTileMutex.unlock();

						return mTileMap[i*mTileXCount + j];
					}
				}
			}
		}
		mTileMutex.unlock();
		return nullptr;
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