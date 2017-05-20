#include "RenderContext.h"
#include "RenderThread.h"
#include "RenderThreadContext.h"
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
#include "math/Generator.h"

#include "material/Material.h"

#ifndef PR_NO_GPU
# include "gpu/GPU.h"
#endif

namespace PR
{
	RenderContext::RenderContext(uint32 index, uint32 ox, uint32 oy, uint32 w, uint32 h, uint32 fw, uint32 fh,
		const Scene& scene, const std::string& workingDir, GPU* gpu, const RenderSettings& settings) :
		mIndex(index), mOffsetX(ox), mOffsetY(oy), mWidth(w), mHeight(h), mFullWidth(fw), mFullHeight(fh),
		mWorkingDir(workingDir),
		mCamera(scene.activeCamera()), mScene(scene),
		mOutputMap(nullptr),
		mTileWidth(w/8), mTileHeight(h/8), mTileXCount(8), mTileYCount(8),
		mTileMap(nullptr), mIncrementalCurrentSample(0),
		mRenderSettings(settings), mGPU(gpu), mIntegrator(nullptr), mShouldStop(false)
	{
		PR_ASSERT(mCamera, "Given camera has to be valid");

		reset();

		mOutputMap = new OutputMap(this);
	}

	RenderContext::~RenderContext()
	{
		reset();

		PR_ASSERT(mOutputMap, "OutputMap has to be non zero before deconstruction");
		delete mOutputMap;
	}

	void RenderContext::reset()
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

	static Sampler* createSampler(SamplerMode mode, Random& random, uint32 samples)
	{
		switch (mode)
		{
		case SM_Random:
			return new RandomSampler(random);
		case SM_Uniform:
			return new UniformSampler(random, samples);
		case SM_Jitter:
			return new StratifiedSampler(random, samples);
		default:
		case SM_MultiJitter:
			return new MultiJitteredSampler(random, samples);
		case SM_HaltonQMC:
			return new HaltonQMCSampler(samples);
		}
	}

	void RenderContext::start(uint32 tcx, uint32 tcy, int32 threads)
	{
		reset();

		/* Setup entities */
		for (const auto& entity : mScene.renderEntities())
		{
			if(entity->isLight())
				mLights.push_back(entity.get());
		}

		/* Setup integrators */
		if (mRenderSettings.debugMode() != DM_None)
		{
			mIntegrator = new DebugIntegrator(this);
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
				mIntegrator = new BiDirectIntegrator(this);
				break;
			case IM_PPM:
				mIntegrator = new PPMIntegrator(this);
				break;
			}
		}

		PR_ASSERT(mIntegrator, "Integrator should be set after selection");

		PR_LOGGER.log(L_Info, M_Scene, "Rendering with: ");
		PR_LOGGER.logf(L_Info, M_Scene, "  AA Samples: %i",  mRenderSettings.maxAASampleCount());
		PR_LOGGER.logf(L_Info, M_Scene, "  Lens Samples: %i",  mRenderSettings.maxLensSampleCount());
		PR_LOGGER.logf(L_Info, M_Scene, "  Time Samples: %i",  mRenderSettings.maxTimeSampleCount());
		PR_LOGGER.logf(L_Info, M_Scene, "  Spectral Samples: %i",  mRenderSettings.maxSpectralSampleCount());

		/* Setup threads */
		uint32 threadCount = Thread::hardwareThreadCount();
		if (threads < 0)
			threadCount = std::max(1, (int32)threadCount + threads);
		else if(threads > 0)
			threadCount = threads;

		for (uint32 i = 0; i < threadCount; ++i)
		{
			RenderThread* thread = new RenderThread(this, i);
			mThreads.push_back(thread);

			RenderThreadContext& context = thread->context();
			context.setAASampler(createSampler(
				mRenderSettings.aaSampler(), context.random(), mRenderSettings.maxAASampleCount()));
			context.setLensSampler(createSampler(
				mRenderSettings.lensSampler(), context.random(), mRenderSettings.maxLensSampleCount()));
			context.setTimeSampler(createSampler(
				mRenderSettings.timeSampler(), context.random(), mRenderSettings.maxTimeSampleCount()));
			context.setSpectralSampler(createSampler(
				mRenderSettings.spectralSampler(), context.random(), mRenderSettings.maxSpectralSampleCount()));
		}

		mIntegrator->init();
		mOutputMap->init();

		// Calculate tile sizes, etc.
		mTileXCount = std::max<uint32>(1,tcx);
		mTileYCount = std::max<uint32>(1,tcy);
		mTileWidth = (uint32)std::ceil(mWidth / (float)mTileXCount);
		mTileHeight = (uint32)std::ceil(mHeight / (float)mTileYCount);
		mTileMap = new RenderTile*[mTileXCount*mTileYCount];

		switch(mRenderSettings.tileMode())
		{
		default:
		case TM_Linear:
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for (uint32 j = 0; j < mTileXCount; ++j)
				{
					uint32 sx = j*mTileWidth;
					uint32 sy = i*mTileHeight;
					mTileMap[i*mTileXCount + j] = new RenderTile(
						sx,
						sy,
						std::min(mWidth, sx + mTileWidth),
						std::min(mHeight, sy + mTileHeight));
				}
			}
			break;
		case TM_Tile:
		{
			uint32 k = 0;
			// Even
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for(uint32 j = (i%2 ? 1 : 0); j < mTileXCount; j+=2)
				{
					uint32 sx = j*mTileWidth;
					uint32 sy = i*mTileHeight;

					mTileMap[k] = new RenderTile(
						sx,
						sy,
						std::min(mWidth, sx + mTileWidth),
						std::min(mHeight, sy + mTileHeight));
					++k;
				}
			}
			// Odd
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for(uint32 j = (i%2 ? 0 : 1); j < mTileXCount; j += 2)
				{
					uint32 sx = j*mTileWidth;
					uint32 sy = i*mTileHeight;

					mTileMap[k] = new RenderTile(
						sx,
						sy,
						std::min(mWidth, sx + mTileWidth),
						std::min(mHeight, sy + mTileHeight));
					++k;
				}
			}
		}
			break;
		case TM_Spiral:
		{
			MinRadiusGenerator<2> generator(std::max(mTileXCount/2, mTileYCount/2));
			uint32 i = 0;
			while(generator.hasNext())
			{
				const auto p = generator.next();
				const auto tx = mTileXCount/2 + p[0];
				const auto ty = mTileYCount/2 + p[1];

				if(tx >= 0 && tx < mTileXCount &&
					ty >= 0 && ty < mTileYCount)
				{
					mTileMap[i] = new RenderTile(
						tx*mTileWidth,
						ty*mTileHeight,
					std::min(mWidth, tx*mTileWidth + mTileWidth),
					std::min(mHeight, ty*mTileHeight + mTileHeight));
					++i;
				}
			}
		}
			break;
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

	void RenderContext::render(RenderThreadContext* context, const Eigen::Vector2i& pixel,
		uint32 sample, uint32 pass)
	{
		PR_ASSERT(mOutputMap, "OutputMap has to be initialized before rendering");
		PR_ASSERT(context, "Rendering needs a valid context");

		if (mRenderSettings.isIncremental())// Only one sample a time!
		{
			renderIncremental(context, pixel, sample, pass);
		}
		else// Everything
		{
			const uint32 SampleCount = mRenderSettings.maxCameraSampleCount();

			for (uint32 currentSample = sample;
				currentSample < SampleCount && !mOutputMap->isPixelFinished(pixel);
				++currentSample)
			{
				renderIncremental(context, pixel, currentSample, pass);
			}
		}
	}
	void RenderContext::renderIncremental(RenderThreadContext* context, const Eigen::Vector2i& pixel,
		uint32 sample, uint32 pass)
	{
		if(mOutputMap->isPixelFinished(pixel))
			return;

		ShaderClosure sc;
		//const auto aaM = mRenderSettings.maxAASampleCount();
		const auto lensM = mRenderSettings.maxLensSampleCount();
		const auto timeM = mRenderSettings.maxTimeSampleCount();
		const auto spectralM = mRenderSettings.maxSpectralSampleCount();

		const auto aasample = sample/(lensM*timeM*spectralM);
		const auto lenssample = (sample%(lensM*timeM*spectralM))/(timeM*spectralM);
		const auto timesample = (sample%(timeM*spectralM))/spectralM;
		const auto spectralsample = sample%spectralM;

		const auto aa = context->aaSampler()->generate2D(aasample);
		const auto lens = context->lensSampler()->generate2D(lenssample);
		auto time = context->timeSampler()->generate1D(timesample);
		switch(mRenderSettings.timeMappingMode())
		{
		default:
		case TMM_Center:
			time -= 0.5;
			break;
		case TMM_Right:
			break;
		case TMM_Left:
			time *= -1;
			break;
		}
		time *= mRenderSettings.timeScale();

		uint8 specInd = std::min<uint8>(Spectrum::SAMPLING_COUNT-1,
			std::floor(
				context->spectralSampler()->generate1D(spectralsample) * Spectrum::SAMPLING_COUNT));

		const Spectrum spec = renderSample(context,
			pixel(0) + aa(0) - 0.5f, pixel(1) + aa(1) - 0.5f,
			lens(0), lens(1),
			time, specInd,
			pass,
			sc);

		mOutputMap->pushFragment(pixel,spec,sc);
	}

	Spectrum RenderContext::renderSample(RenderThreadContext* context,
		float x, float y, float rx, float ry, float t, uint8 wavelength,
		uint32 pass, ShaderClosure& sc)
	{
		context->stats().incPixelSampleCount();

		// To camera coordinates [-1,1]
		//float nx = 2 * (x / mWidth - 0.5f);
		//float ny = 2 * (y / mHeight - 0.5f);
		const float fnx = 2 * ((x+mOffsetX) / mFullWidth - 0.5f);
		const float fny = 2 * ((y+mOffsetY) / mFullHeight - 0.5f);

		Ray ray = mCamera->constructRay(fnx, fny, rx, ry, t, wavelength);
		ray.setPixel(Eigen::Vector2i(
			(uint32)std::max(0.0f, std::round(x)),
			(uint32)std::max(0.0f, std::round(y))
			));

		return mIntegrator->apply(ray, context, pass, sc);
	}

	std::list<RenderTile> RenderContext::currentTiles() const
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

	RenderEntity* RenderContext::shoot(const Ray& ray, ShaderClosure& sc, RenderThreadContext* context)
	{
		if (ray.depth() < mRenderSettings.maxRayDepth())
		{
			sc.Flags = 0;

			FaceSample fs;
			RenderEntity* entity = mScene.checkCollision(ray, fs);
			sc = fs;

			sc.NgdotV = ray.direction().dot(sc.Ng);
			sc.N = Reflection::faceforward(sc.NgdotV, sc.Ng);
			sc.Flags |= Reflection::is_inside(sc.NgdotV) ? SCF_Inside : 0;
			sc.NdotV = -std::abs(sc.NgdotV);
			sc.V = ray.direction();
			sc.T = ray.time();
			sc.WavelengthIndex = ray.wavelength();
			sc.Depth2 = (ray.startPosition() - sc.P).squaredNorm();

			if(entity)
				sc.EntityID = entity->id();

			if (sc.Flags & SCF_Inside)
			{
				sc.Nx = -fs.Nx;
				//sc.Ny = -fs.Ny;
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

	bool RenderContext::shootForDetection(const Ray& ray, RenderThreadContext* context)
	{
		if (ray.depth() < mRenderSettings.maxRayDepth())
		{
			FaceSample fs;
			bool found = mScene.checkIfCollides(ray, fs);

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

	RenderEntity* RenderContext::shootWithEmission(Spectrum& appliedSpec, const Ray& ray,
		ShaderClosure& sc, RenderThreadContext* context)
	{
		if (ray.depth() >= mRenderSettings.maxRayDepth())
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

			for(const auto& e : mScene.infiniteLights())
				appliedSpec += e->apply(ray.direction());

			if(context)
				context->stats().incBackgroundHitCount();
			else
				mGlobalStatistics.incBackgroundHitCount();
		}

		return entity;
	}

	void RenderContext::waitForNextPass()
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

	uint32 RenderContext::threads() const
	{
		return (uint32)mThreads.size();
	}

	bool RenderContext::isFinished()
	{
		for (RenderThread* thread : mThreads)
		{
			if (thread->state() != Thread::S_Stopped)
				return false;
		}

		return true;
	}

	void RenderContext::waitForFinish()
	{
		while (!isFinished())
			std::this_thread::yield();
	}

	void RenderContext::stop()
	{
		mShouldStop = true;

		for (RenderThread* thread : mThreads)
			thread->stop();
	}

	RenderTile* RenderContext::getNextTile()
	{
		const auto maxCameraSamples = mRenderSettings.maxCameraSampleCount();
		mTileMutex.lock();
		if (mRenderSettings.isIncremental())
		{
			for (uint32 i = 0; i < mTileYCount; ++i)
			{
				for (uint32 j = 0; j < mTileXCount; ++j)
				{
					// TODO: Better check up for AS
					if ( mTileMap[i*mTileXCount + j]->samplesRendered() <= mIncrementalCurrentSample &&
						mTileMap[i*mTileXCount + j]->samplesRendered() < maxCameraSamples &&
						!mTileMap[i*mTileXCount + j]->isWorking())
					{
						mTileMap[i*mTileXCount + j]->setWorking(true);
						mTileMutex.unlock();

						return mTileMap[i*mTileXCount + j];
					}
				}
			}

			if (mIncrementalCurrentSample < maxCameraSamples)// Try again
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
						mTileMap[i*mTileXCount + j]->samplesRendered() < maxCameraSamples &&
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

	const std::list<RenderEntity*>& RenderContext::lights() const
	{
		return mLights;
	}

	RenderStatistics RenderContext::statistics() const
	{
		RenderStatistics s = mGlobalStatistics;
		for (RenderThread* thread : mThreads)
			s += thread->context().stats();
		return s;
	}

	RenderStatus RenderContext::status() const
	{
		RenderStatistics s = statistics();

		RenderStatus status = mIntegrator->status();
		status.setField("global.ray_count", s.rayCount());
		status.setField("global.pixel_sample_count", s.pixelSampleCount());
		status.setField("global.entity_hit_count", s.entityHitCount());
		status.setField("global.background_hit_count", s.backgroundHitCount());
		return status;
	}

	void RenderContext::onNextPass()
	{
		for (uint32 i = 0; i < mTileYCount; ++i)
			for (uint32 j = 0; j < mTileXCount; ++j)
				mTileMap[i*mTileXCount + j]->reset();

		bool clear = false;
		mIntegrator->onNextPass(mCurrentPass + 1, clear);

		if(clear)
			mOutputMap->clear();
	}
}
