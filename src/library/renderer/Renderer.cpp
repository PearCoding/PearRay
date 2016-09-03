#include "Renderer.h"
#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "RenderStatistics.h"
#include "DisplayDriver.h"

#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"

#include "integrator/BiDirectIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/DebugIntegrator.h"

#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/UniformSampler.h"
#include "sampler/HaltonQMCSampler.h"

#include "shader/SamplePoint.h"

#include "Logger.h"
#include "math/Reflection.h"
#include "math/Projection.h"

#include "material/Material.h"

#ifndef PR_NO_GPU
# include "gpu/GPU.h"
#endif

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene, bool useGPU) :
		mWidth(w), mHeight(h),
		mCamera(cam), mScene(scene),
		mResult(nullptr), mBackgroundMaterial(nullptr),
		mTileWidth(w/8), mTileHeight(h/8), mTileXCount(8), mTileYCount(8),
		mIncrementalCurrentSample(0), mTileMap(nullptr),
		mGPU(nullptr), mIntegrator(nullptr)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		// Setup GPU
#ifndef PR_NO_GPU
		if(useGPU)
		{
			mGPU = new GPU();
			if (!mGPU->init(""))
			{
				delete mGPU;
				mGPU = nullptr;
			}
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

		if(mIntegrator)
		{
			delete mIntegrator;
			mIntegrator = nullptr;
		}

		mLights.clear();
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
		return (uint32)std::ceil((mRenderSettings.cropMaxX() - mRenderSettings.cropMinX()) * mWidth);
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
		return (uint32)std::ceil((mRenderSettings.cropMaxY() - mRenderSettings.cropMinY()) * mHeight);
	}

	uint32 Renderer::cropPixelOffsetX() const
	{
		return std::ceil(mRenderSettings.cropMinX() * mWidth);
	}

	uint32 Renderer::cropPixelOffsetY() const
	{
		return std::ceil(mRenderSettings.cropMinY() * mHeight);
	}

	void Renderer::setBackgroundMaterial(Material* m)
	{
		mBackgroundMaterial = m;
	}

	Material* Renderer::backgroundMaterial() const
	{
		return mBackgroundMaterial;
	}

	void Renderer::start(IDisplayDriver* display, uint32 tcx, uint32 tcy, int32 threads)
	{
		PR_ASSERT(display);

		reset();

		if(mResult != display)
			display->init(this);
		
		mResult = display;

		/* Setup entities */
		for (RenderEntity* entity : mScene->renderEntities())
		{
			if(entity->isLight())
				mLights.push_back(entity);
		}

		/* Setup integrators */
		if (mRenderSettings.debugMode() != DM_None)
		{
			mIntegrator = new DebugIntegrator();
		}
		else
		{
			if (mRenderSettings.maxLightSamples() == 0)
			{
				PR_LOGGER.log(L_Warning, M_Scene, "MaxLightSamples is zero: Nothing to render");
				return;
			}

			switch(mRenderSettings.integratorMode())
			{
			case IM_Direct:
				mIntegrator = new DirectIntegrator(this);
				break;
			default:
			case IM_BiDirect:
				mIntegrator = new BiDirectIntegrator();
				break;
			case IM_PPM:
				mIntegrator = nullptr;// TODO
				break;
			}
		}

		PR_ASSERT(mIntegrator);
		
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
			case SM_HaltonQMC:
				context.setPixelSampler(new HaltonQMCSampler(mRenderSettings.maxPixelSampleCount()));
				break;
			}
		}

		mIntegrator->init(this);

		// Calculate tile sizes, etc.

		mTileXCount = std::max<uint32>(1,tcx);
		mTileYCount = std::max<uint32>(1,tcy);
		mTileWidth = (uint32)std::ceil(renderWidth() / (float)mTileXCount);
		mTileHeight = (uint32)std::ceil(renderHeight() / (float)mTileYCount);
		mTileMap = new RenderTile*[mTileXCount*mTileYCount];
		for (uint32 i = 0; i < mTileYCount; ++i)
		{
			for (uint32 j = 0; j < mTileXCount; ++j)
			{
				uint32 sx = cropPixelOffsetX() + j*mTileWidth;
				uint32 sy = cropPixelOffsetY() + i*mTileHeight;
				mTileMap[i*mTileXCount + j] = new RenderTile(
					sx,
					sy,
					PM::pm_MinT((uint32)std::ceil(mRenderSettings.cropMaxX()*mWidth), sx + mTileWidth),
					PM::pm_MinT((uint32)std::ceil(mRenderSettings.cropMaxY()*mHeight), sy + mTileHeight));
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
		PR_ASSERT(mResult);
		PR_ASSERT(context);

		if (mRenderSettings.isIncremental())// Only one sample a time!
		{
			auto s = context->pixelSampler()->generate2D(sample);

			float rx = context->random().getFloat();// Random sampling
			float ry = context->random().getFloat();
			
			Spectrum spec = renderSample(context, x + PM::pm_GetX(s) - 0.5f, y + PM::pm_GetY(s) - 0.5f,
				rx, ry,//TODO
				0.0f);//TODO

			mResult->pushFragment(x, y, 0, sample, spec);
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

			mResult->pushFragment(x, y, 0, 0, newSpec / (float)SampleCount);
		}
	}

	Spectrum Renderer::renderSample(RenderContext* context, float x, float y, float rx, float ry, float t)
	{
		context->stats().incPixelSampleCount();
		
		Ray ray = mCamera->constructRay(2 * (x + 0.5f) / (float)mWidth - 1.0f,
			2 * (y + 0.5f) / (float)mHeight - 1.0f,// To camera coordinates [-1,1]
			rx, ry, t);

		return mIntegrator->apply(ray, context);
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

	RenderEntity* Renderer::shoot(const Ray& ray, SamplePoint& collisionPoint, RenderContext* context, RenderEntity* ignore)
	{
		const uint32 maxDepth = (ray.maxDepth() == 0) ?
			mRenderSettings.maxRayDepth() : PM::pm_MinT<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() < maxDepth)
		{
			collisionPoint.Flags = 0;

			RenderEntity* entity = mScene->checkCollision(ray, collisionPoint, ignore);

			const float NdotV = PM::pm_Dot3D(ray.direction(), collisionPoint.Ng);
			collisionPoint.N = Reflection::faceforward(NdotV, collisionPoint.Ng);
			collisionPoint.Flags |= (NdotV > 0) ? SPF_Inside : 0;
			collisionPoint.NdotV = std::abs(NdotV);
			collisionPoint.V = ray.direction();

			if (collisionPoint.Flags & SPF_Inside)
			{
				collisionPoint.Nx = PM::pm_Negate(collisionPoint.Nx);
				collisionPoint.Ny = PM::pm_Negate(collisionPoint.Ny);
			}

			context->stats().incRayCount();
			if(!entity)
				context->stats().incEntityHitCount();
			
			return entity;
		}
		else
		{
			return nullptr;
		}
	}

	RenderEntity* Renderer::shootWithEmission(Spectrum& appliedSpec, const Ray& ray,
			SamplePoint& collisionPoint, RenderContext* context, RenderEntity* ignore)
	{
		RenderEntity* entity = shoot(ray, collisionPoint, context, ignore);
		if (entity)
		{
			if(collisionPoint.Material && collisionPoint.Material->emission())
			{
				//const float a = entity->surfaceArea(collisionPoint.Material);
				appliedSpec += collisionPoint.Material->emission()->eval(collisionPoint); // TODO
			}
		}
		else if (mBackgroundMaterial)
		{
			SamplePoint point;
			point.Material = mBackgroundMaterial;
			point.Flags = SPF_Inside;
			point.V = ray.direction();
			point.Ng = ray.direction();
			point.N = PM::pm_Negate(ray.direction());
			point.P = ray.direction();//Radius?

			Projection::tangent_frame(point.N, point.Nx, point.Ny);
			point.UV = Projection::sphereUV(ray.direction());

			appliedSpec += mBackgroundMaterial->emission()->eval(point) /** (4*PM_INV_PI_F)*/;

			context->stats().incBackgroundHitCount();
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

	const std::list<RenderEntity*>& Renderer::lights() const
	{
		return mLights;
	}

	RenderStatistics Renderer::stats(RenderThread* thread) const
	{
		if(thread == nullptr)// All
		{
			RenderStatistics s;
			for (RenderThread* thread : mThreads)
				s += thread->context().stats();

			return s;
		}
		else
		{
			return thread->context().stats();
		}
	}
}
