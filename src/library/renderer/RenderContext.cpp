#include "RenderContext.h"
#include "Logger.h"
#include "Profiler.h"
#include "RenderThread.h"
#include "RenderTile.h"
#include "RenderTileMap.h"
#include "RenderTileSession.h"
#include "buffer/OutputBuffer.h"
#include "camera/ICamera.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "scene/Scene.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderContext::RenderContext(uint32 index, const Point2i& viewOffset, const Size2i& viewSize,
							 const std::shared_ptr<IIntegrator>& integrator,
							 const std::shared_ptr<Scene>& scene,
							 const std::shared_ptr<SpectrumDescriptor>& specDesc,
							 const RenderSettings& settings)
	: mIndex(index)
	, mViewOffset(viewOffset)
	, mViewSize(viewSize)
	, mScene(scene)
	, mSpectrumDescriptor(specDesc)
	, mOutputMap()
	, mEmissiveSurfaceArea(0.0f)
	, mTileMap()
	, mThreadsWaitingForIteration(0)
	, mIncrementalCurrentIteration(0)
	, mRenderSettings(settings)
	, mIntegrator(integrator)
	, mShouldStop(false)
{
	PR_ASSERT(mIntegrator, "Integrator can not be NULL!");
	PR_ASSERT(mScene, "Scene can not be NULL!");
	PR_ASSERT(mSpectrumDescriptor, "Spectrum Descriptor can not be NULL!");
	reset();

	mOutputMap = std::make_unique<OutputBuffer>(
		settings.createPixelFilter(),
		viewSize, mSpectrumDescriptor->samples());
}

RenderContext::~RenderContext()
{
	reset();
}

void RenderContext::reset()
{
	PR_PROFILE_THIS;

	for (RenderThread* thread : mThreads)
		delete thread;

	mShouldStop					 = false;
	mThreadsWaitingForIteration	 = 0;
	mThreadsWaitingForPass		 = 0;
	mCurrentPass				 = 0;
	mIncrementalCurrentIteration = 0;

	mThreads.clear();
	mLights.clear();
}

void RenderContext::start(uint32 rtx, uint32 rty, int32 threads)
{
	PR_PROFILE_THIS;

	reset();

	PR_ASSERT(mOutputMap, "Output Map must be already created!");

	mScene->beforeRender(this);

	/* Setup entities */
	mEmissiveSurfaceArea = 0.0f;
	for (auto entity : mScene->entities()) {
		if (entity->isLight()) {
			mEmissiveSurfaceArea += entity->surfaceArea();
			mLights.push_back(entity);
		}
	}

	// Get other informations
	PR_LOG(L_INFO) << "Rendering with: " << std::endl;
	PR_LOG(L_INFO) << "  Lights: " << mLights.size() << std::endl;
	PR_LOG(L_INFO) << "  Emissive Area: " << mEmissiveSurfaceArea << std::endl;

	if (mEmissiveSurfaceArea < PR_EPSILON)
		mEmissiveSurfaceArea = 1;

	/* Setup threads */
	uint32 threadCount = Thread::hardwareThreadCount();
	if (threads < 0)
		threadCount = std::max(1, static_cast<int32>(threadCount) + threads);
	else if (threads > 0)
		threadCount = threads;

	for (uint32 i = 0; i < threadCount; ++i) {
		RenderThread* thread = new RenderThread(i, this);
		mThreads.push_back(thread);
	}

	mTileMap = std::make_unique<RenderTileMap>();
	mTileMap->init(*this, rtx, rty, mRenderSettings.tileMode);

	// Init modules
	mIntegrator->onInit(this);

	mOutputMap->clear();

	// Start
	mIntegrator->onStart();
	if (mIntegrator->needNextPass(0)) {
		bool clear; // Doesn't matter, as it is already clean.
		mIntegrator->onNextPass(0, clear);
	}

	PR_LOG(L_INFO) << "Rendering with " << threadCount << " threads and " << rtx << "x" << rty << " tiles." << std::endl;
	PR_LOG(L_INFO) << "Starting threads." << std::endl;
	for (RenderThread* thread : mThreads)
		thread->start();
}

void RenderContext::notifyEnd()
{
	PR_PROFILE_THIS;

	if (isFinished() && mIntegrator)
		mIntegrator->onEnd();

	mScene->afterRender(this);
}

size_t RenderContext::tileCount() const
{
	return mTileMap->tileCount();
}

const Size2i& RenderContext::maxTileSize() const
{
	return mTileMap->maxTileSize();
}

std::vector<Rect2i> RenderContext::currentTiles() const
{
	PR_PROFILE_THIS;

	std::lock_guard<std::mutex> guard(mTileMutex);

	std::vector<Rect2i> list;
	for (RenderThread* thread : mThreads) {
		RenderTile* tile = thread->currentTile();
		if (tile)
			list.push_back(Rect2i(tile->start(), tile->viewSize()));
	}
	return list;
}

void RenderContext::waitForNextPass()
{
	PR_PROFILE_THIS;

	std::unique_lock<std::mutex> lk(mPassMutex);
	mThreadsWaitingForPass++;

	if (mThreadsWaitingForPass == threads()) {
		if (mIntegrator->needNextPass(mCurrentPass + 1))
			onNextPass();

		mCurrentPass++;
		mThreadsWaitingForPass = 0;
		lk.unlock();

		mPassCondition.notify_all();
	} else {
		mPassCondition.wait(lk, [this] { return mShouldStop || mThreadsWaitingForPass == 0; });
		lk.unlock();
	}
}

bool RenderContext::isFinished() const
{
	PR_PROFILE_THIS;

	for (RenderThread* thread : mThreads) {
		if (thread->state() != Thread::S_Stopped)
			return false;
	}

	return true;
}

void RenderContext::waitForFinish()
{
	PR_PROFILE_THIS;

	while (!isFinished())
		std::this_thread::yield();
}

void RenderContext::stop()
{
	PR_PROFILE_THIS;

	mShouldStop = true;

	for (RenderThread* thread : mThreads)
		thread->stop();

	// Make sure threads are not inside conditions
	mIterationCondition.notify_all();
	mPassCondition.notify_all();
}

RenderTile* RenderContext::getNextTile()
{
	PR_PROFILE_THIS;

	std::lock_guard<std::mutex> guard(mTileMutex);
	RenderTile* tile = nullptr;

	if (mRenderSettings.useAdaptiveTiling) {
		// Try till we find a tile or all samples of this iteration are already rendered
		while (tile == nullptr && !mTileMap->allFinished()) {
			tile = mTileMap->getNextTile(mIncrementalCurrentIteration);
			if (tile == nullptr) {
				std::unique_lock<std::mutex> lk(mIterationMutex);
				++mThreadsWaitingForIteration;

				if (mThreadsWaitingForIteration == threads()) {
					optimizeTileMap();
					++mIncrementalCurrentIteration;
					mThreadsWaitingForIteration = 0;
					lk.unlock();

					mIterationCondition.notify_all();
				} else {
					mIterationCondition.wait(lk, [this] { return mShouldStop || mThreadsWaitingForIteration == 0 || mTileMap->allFinished(); });
					lk.unlock();
				}
			}
		}

		mIterationCondition.notify_all();
	} else {
		// Try till we find a tile or all samples of this iteration are already rendered
		while (tile == nullptr && !mTileMap->allFinished()) {
			tile = mTileMap->getNextTile(mIncrementalCurrentIteration);
			if (tile == nullptr) {
				++mIncrementalCurrentIteration;
			}
		}
	}

	return tile;
}

void RenderContext::optimizeTileMap()
{
	std::lock_guard<std::mutex> guard(mTileMutex);
	mTileMap->optimize();
}

RenderTileStatistics RenderContext::statistics() const
{
	return mTileMap->statistics();
}

RenderStatus RenderContext::status() const
{
	PR_PROFILE_THIS;

	RenderTileStatistics s = statistics();
	RenderStatus status	   = mIntegrator->status();

	// Approximate percentage if not given by the integrator
	if (status.percentage() < 0)
		status.setPercentage(mTileMap->percentage());

	status.setField("global.ray_count", s.rayCount());
	status.setField("global.camera_ray_count", s.cameraRayCount());
	status.setField("global.light_ray_count", s.lightRayCount());
	status.setField("global.bounce_ray_count", s.bounceRayCount());
	status.setField("global.shadow_ray_count", s.shadowRayCount());
	status.setField("global.pixel_sample_count", s.pixelSampleCount());
	status.setField("global.entity_hit_count", s.entityHitCount());
	status.setField("global.background_hit_count", s.backgroundHitCount());
	status.setField("global.current_pass", (uint64)mCurrentPass);

	return status;
}

void RenderContext::onNextPass()
{
	PR_PROFILE_THIS;

	mTileMap->reset();

	bool clear = false;
	mIntegrator->onNextPass(mCurrentPass + 1, clear);

	mIncrementalCurrentIteration = 0;

	if (clear)
		mOutputMap->clear();
}
} // namespace PR
