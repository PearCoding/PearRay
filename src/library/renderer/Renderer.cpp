#include "Renderer.h"
#include "RenderThread.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "OutputMap.h"

#include "camera/Camera.h"
#include "scene/Scene.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"

#include "integrator/BiDirectIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/PPMIntegrator.h"

#include "light/IInfiniteLight.h"

#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/UniformSampler.h"
#include "sampler/HaltonQMCSampler.h"

#include "shader/FaceSample.h"
#include "shader/ShaderClosure.h"

#include "Logger.h"
#include "math/Reflection.h"
#include "math/Projection.h"
#include "math/MSI.h"

#include "material/Material.h"

#ifndef PR_NO_GPU
# include "gpu/GPU.h"
#endif

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h, Camera* cam, Scene* scene,
		const std::string& workingDir, bool useGPU) :
		mWidth(w), mHeight(h),
		mWorkingDir(workingDir),
		mCamera(cam), mScene(scene),
		mOutputMap(nullptr),
		mTileWidth(w/8), mTileHeight(h/8), mTileXCount(8), mTileYCount(8),
		mTileMap(nullptr), mIncrementalCurrentSample(0),
		mGPU(nullptr), mIntegrator(nullptr), mShouldStop(false)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		reset();

		mOutputMap = new OutputMap(this);

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

		delete mOutputMap;

#ifndef PR_NO_GPU
		if (mGPU)
		{
			delete mGPU;
		}
#endif
	}

	void Renderer::reset()
	{
		mShouldStop = false;
		mThreadsWaitingForPass = 0;
		mCurrentPass = 0;
		mIncrementalCurrentSample = 0;

		if(mIntegrator)
		{
			delete mIntegrator;
			mIntegrator = nullptr;
		}

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

	void Renderer::start(uint32 tcx, uint32 tcy, int32 threads)
	{
		reset();

		if(mRenderSettings.isAdaptiveSampling() && !mOutputMap->getChannel(OutputMap::V_Quality))
			mOutputMap->registerChannel(OutputMap::V_Quality, new Output1D(this));

		mOutputMap->init();

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
			mRenderSettings.setMaxPixelSampleCount(1);// Not that much needed for debug
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
				mIntegrator = new PPMIntegrator();// TODO
				break;
			}
		}

		PR_ASSERT(mIntegrator);

		/* Setup threads */
		uint32 threadCount = Thread::hardwareThreadCount();
		if (threads < 0)
			threadCount = PM::pm_Max(1, (int32)threadCount + threads);
		else if(threads > 0)
			threadCount = threads;

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
					PM::pm_Min((uint32)std::ceil(mRenderSettings.cropMaxX()*mWidth), sx + mTileWidth),
					PM::pm_Min((uint32)std::ceil(mRenderSettings.cropMaxY()*mHeight), sy + mTileHeight));
			}
		}

		mIntegrator->onStart();// TODO: onEnd?
		bool clear;// Doesn't matter, as it is already clean.
		if(mIntegrator->needNextPass(0))
			mIntegrator->onNextPass(0, clear);

		PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", threadCount);
		PR_LOGGER.log(L_Info, M_Scene, "Starting threads.");
		for (RenderThread* thread : mThreads)
			thread->start();
	}

	void Renderer::pushPixel_Normalized(float x, float y, const Spectrum& spec, const ShaderClosure& sc)
	{
		uint32 px = PM::pm_Clamp<uint32>(std::round(x * (mWidth-1)), 0, mWidth-1);
		uint32 py = PM::pm_Clamp<uint32>(std::round(y * (mHeight-1)), 0, mHeight-1);

		mOutputMap->pushFragment(px, py, spec, sc);
	}

	Spectrum Renderer::getPixel_Normalized(float x, float y)
	{
		uint32 px = PM::pm_Clamp<uint32>(std::round(x * (mWidth-1)), 0, mWidth-1);
		uint32 py = PM::pm_Clamp<uint32>(std::round(y * (mHeight-1)), 0, mHeight-1);
		
		return mOutputMap->getFragment(px, py);
	}

	void Renderer::render(RenderContext* context, uint32 x, uint32 y, uint32 sample, uint32 pass)
	{
		PR_ASSERT(mOutputMap);
		PR_ASSERT(context);

		ShaderClosure sc;
		if (mRenderSettings.isIncremental())// Only one sample a time!
		{
			if(!mOutputMap->isPixelFinished(x, y))
			{
				auto s = context->pixelSampler()->generate2D(sample);

				float rx = context->random().getFloat();// Random sampling
				float ry = context->random().getFloat();
				
				Spectrum spec = renderSample(context,
					x + PM::pm_GetX(s) - 0.5f, y + PM::pm_GetY(s) - 0.5f,
					rx, ry,//TODO
					0.0f,
					pass,
					sc);

				mOutputMap->pushFragment(x,y,spec,sc);
			}
		}
		else// Everything
		{
			const uint32 SampleCount = mRenderSettings.maxPixelSampleCount();

			for (uint32 currentSample = sample; currentSample < SampleCount && !mOutputMap->isPixelFinished(x, y); ++currentSample)
			{
				auto s = context->pixelSampler()->generate2D(currentSample);

				float rx = context->random().getFloat();// Random sampling
				float ry = context->random().getFloat();

				Spectrum spec = renderSample(context,
					x + PM::pm_GetX(s) - 0.5f, y + PM::pm_GetY(s) - 0.5f,
					rx, ry,//TODO
					0.0f,
					pass,
					sc);

				mOutputMap->pushFragment(x,y,spec,sc);
			}
		}
	}

	Spectrum Renderer::renderSample(RenderContext* context, float x, float y, float rx, float ry, float t, uint32 pass, ShaderClosure& sc)
	{
		context->stats().incPixelSampleCount();
		
		Ray ray = mCamera->constructRay(2 * (x / mWidth - 0.5f) ,
			2 * (y / mHeight - 0.5f),// To camera coordinates [-1,1]
			rx, ry, t);

		return mIntegrator->apply(ray, context, pass, sc);
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

	RenderEntity* Renderer::shoot(const Ray& ray, ShaderClosure& sc, RenderContext* context)
	{
		const uint32 maxDepth = (ray.maxDepth() == 0) ?
			mRenderSettings.maxRayDepth() : PM::pm_Min<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() < maxDepth)
		{
			sc.Flags = 0;

			FaceSample fs;
			RenderEntity* entity = mScene->checkCollision(ray, fs);
			sc = fs;

			sc.NgdotV = PM::pm_Dot3D(ray.direction(), sc.Ng);
			sc.N = Reflection::faceforward(sc.NgdotV, sc.Ng);
			sc.Flags |= (sc.NgdotV > 0) ? SCF_Inside : 0;
			sc.NdotV = std::abs(sc.NgdotV);
			sc.V = ray.direction();
			sc.T = ray.time();
			sc.Depth2 = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(ray.startPosition(), sc.P));
			
			if(entity)
				sc.EntityID = entity->id(); 

			if (sc.Flags & SCF_Inside)
			{
				sc.Nx = PM::pm_Negate(fs.Nx);
				//sc.Ny = PM::pm_Negate(fs.Ny);
			}

			if(context)
			{
				context->stats().incRayCount();
				if(entity)
					context->stats().incEntityHitCount();
			}
			else
			{
				mGlobalStatistics.incRayCount();
				if(entity)
					mGlobalStatistics.incEntityHitCount();
			}
			
			return entity;
		}
		else
		{
			return nullptr;
		}
	}

	bool Renderer::shootForDetection(const Ray& ray, RenderContext* context)
	{
		const uint32 maxDepth = (ray.maxDepth() == 0) ?
			mRenderSettings.maxRayDepth() : PM::pm_Min<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() < maxDepth)
		{
			FaceSample fs;
			bool found = mScene->checkIfCollides(ray, fs);

			if(context)
			{
				context->stats().incRayCount();
				if(found)
					context->stats().incEntityHitCount();
			}
			else
			{
				mGlobalStatistics.incRayCount();
				if(found)
					mGlobalStatistics.incEntityHitCount();
			}
			
			return found;
		}
		else
		{
			return false;
		}
	}

	RenderEntity* Renderer::shootWithEmission(Spectrum& appliedSpec, const Ray& ray,
		ShaderClosure& sc, RenderContext* context)
	{
		const uint32 maxDepth = (ray.maxDepth() == 0) ?
			mRenderSettings.maxRayDepth() : PM::pm_Min<uint32>(mRenderSettings.maxRayDepth() + 1, ray.maxDepth());
		if (ray.depth() >= maxDepth)
			return nullptr;
		
		RenderEntity* entity = shoot(ray, sc, context);
		if (entity)
		{
			if(sc.Material && sc.Material->emission())
				appliedSpec = sc.Material->emission()->eval(sc);
			else
				appliedSpec.clear();
		}
		else
		{
			appliedSpec.clear();
			
			for(IInfiniteLight* e : mScene->infiniteLights())
				appliedSpec += e->apply(ray.direction());

			if(context)
				context->stats().incBackgroundHitCount();
			else
				mGlobalStatistics.incBackgroundHitCount();
		}

		return entity;
	}
	
	void Renderer::waitForNextPass()
	{
		std::unique_lock<std::mutex> lk(mPassMutex);
		mThreadsWaitingForPass++;

		if(mThreadsWaitingForPass == threads())
		{
			if(mIntegrator->needNextPass(mCurrentPass + 1))
				onNextPass();
			
			mCurrentPass++;
			mThreadsWaitingForPass = 0;
			lk.unlock();

			mPassCondition.notify_all();
		}
		else
		{
			mPassCondition.wait(lk, [this]{ return mShouldStop || mThreadsWaitingForPass == 0; });
			lk.unlock();
		}
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
				return false;
		}

		return true;
	}

	void Renderer::waitForFinish()
	{
		while (!isFinished())
			std::this_thread::yield();
	}

	void Renderer::stop()
	{
		mShouldStop = true;

		for (RenderThread* thread : mThreads)
			thread->stop();
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
					// TODO: Better check up for AS
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

	RenderStatistics Renderer::stats() const
	{
		RenderStatistics s = mGlobalStatistics;
		for (RenderThread* thread : mThreads)
			s += thread->context().stats();
		return s;
	}
	
	float Renderer::percentFinished() const
	{
		PR_ASSERT(mIntegrator);

		const uint64 maxPasses = mIntegrator->maxPasses(this);
		const uint64 maxSamples = mIntegrator->maxSamples(this);
		const uint64 pixelsFinished = mOutputMap->finishedPixelCount();
		RenderStatistics s = stats();

		double finishedPercent = mRenderSettings.isIncremental() ? pixelsFinished / (double)(renderWidth()*renderHeight()) : 0;
		double unfinishedPercent = (1-finishedPercent)*(s.pixelSampleCount() / (double)maxSamples);
		return 100 * static_cast<float>(
				(mCurrentPass + finishedPercent + unfinishedPercent) / (double)maxPasses);
	}

	void Renderer::onNextPass()
	{
		for (uint32 i = 0; i < mTileYCount; ++i)
		{
			for (uint32 j = 0; j < mTileXCount; ++j)
			{
				mTileMap[i*mTileXCount + j]->reset();
			}
		}

		bool clear = false;
		mIntegrator->onNextPass(mCurrentPass + 1, clear);

		if(clear)
			mOutputMap->clear();
	}
}
