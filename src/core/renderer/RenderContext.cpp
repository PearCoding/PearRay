#include "RenderContext.h"
#include "Logger.h"
#include "Platform.h"
#include "Profiler.h"
#include "RenderThread.h"
#include "RenderTile.h"
#include "RenderTileMap.h"
#include "RenderTileSession.h"
#include "buffer/FrameBufferSystem.h"
#include "camera/ICamera.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "scene/Scene.h"
#include "trace/IntersectionPoint.h"

namespace PR {
RenderContext::RenderContext(uint32 index, const Point2i& viewOffset, const Size2i& viewSize,
							 const std::shared_ptr<IIntegrator>& integrator,
							 const std::shared_ptr<Scene>& scene,
							 const RenderSettings& settings)
	: mIndex(index)
	, mViewOffset(viewOffset)
	, mViewSize(viewSize)
	, mMaxIterationCount(settings.maxSampleCount())
	, mScene(scene)
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
	reset();

	mOutputMap = std::make_unique<FrameBufferSystem>(
		settings.createPixelFilter(),
		viewSize, 3, settings.spectralMono); // TODO: Should be encapsulated by an output device
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
	mIncrementalCurrentIteration = 0;

	mThreads.clear();
	mLights.clear();
}

void RenderContext::start(uint32 rtx, uint32 rty, int32 threads)
{
	PR_PROFILE_THIS;

	setupFloatingPointEnvironment();

	reset();

	PR_ASSERT(mOutputMap, "Output Map must be already created!");

	/* Setup entities */
	mEmissiveSurfaceArea = 0.0f;
	for (auto entity : mScene->entities()) {
		if (entity->isLight()) {
			mEmissiveSurfaceArea += entity->worldSurfaceArea();
			mLights.push_back(entity);
		}
	}

	if (mEmissiveSurfaceArea < PR_EPSILON)
		mEmissiveSurfaceArea = 0.0f;

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

	// Call all interested objects after thread count is fixed
	mScene->beforeRender(this);

	mTileMap = std::make_unique<RenderTileMap>();
	mTileMap->init(*this, rtx, rty, mRenderSettings.tileMode);

	// Init modules
	mIntegrator->onInit(this);
	mOutputMap->clear();

	// Get other informations
	PR_LOG(L_INFO) << "Rendering with:    " << std::endl
				   << "  Threads:         " << threadCount << std::endl
				   << "  Tiles:           " << rtx << " x " << rty << std::endl
				   << "  Entities:        " << mScene->entities().size() << std::endl
				   << "  Lights:          " << mLights.size() << std::endl
				   << "  Materials:       " << mScene->materials().size() << std::endl
				   << "  Emissions:       " << mScene->emissions().size() << std::endl
				   << "  InfLights:       " << mScene->infiniteLights().size() << std::endl
				   << "  Emissive Area:   " << mEmissiveSurfaceArea << std::endl
				   << "  Scene Extent:    " << mScene->boundingBox().width() << " x " << mScene->boundingBox().height() << " x " << mScene->boundingBox().depth() << std::endl
				   << "  Spectral Domain: [" << mRenderSettings.spectralStart << ", " << mRenderSettings.spectralEnd << "]" << std::endl
				   << "  Max Iterations:  " << mMaxIterationCount << std::endl;

	// Start
	mIntegrator->onStart();
	if (mIterationCallback)
		mIterationCallback(mIncrementalCurrentIteration);
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

Size2i RenderContext::maxTileSize() const
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

	/*while (!isFinished())
		std::this_thread::yield();*/

	// Wait for all threads to stop
	for (RenderThread* thread : mThreads)
		thread->join();
}

void RenderContext::requestStop()
{
	PR_PROFILE_THIS;

	mShouldStop = true;

	// Request all threads to stop
	for (RenderThread* thread : mThreads)
		thread->requestStop();

	// Make sure threads are not inside conditions
	mIterationCondition.notify_all();
}

void RenderContext::stop()
{
	PR_PROFILE_THIS;

	requestStop();

	// Wait for all threads to stop
	for (RenderThread* thread : mThreads)
		thread->join();
}

RenderTile* RenderContext::getNextTile()
{
	PR_PROFILE_THIS;

	const auto threadCount = threads();

	RenderTile* tile		  = nullptr;
	bool breakBecauseFinished = false;

	// Try till we find a tile to render
	while (tile == nullptr) {
		tile = mTileMap->getNextTile(mIncrementalCurrentIteration);
		if (tile == nullptr) { // If no tile on this iteration was found sync all threads
			std::unique_lock<std::mutex> lk(mIterationMutex);
			++mThreadsWaitingForIteration;

			if (mThreadsWaitingForIteration == threadCount) { // The last thread arriving here has the honor to call optional callbacks and initiate the next iteration
				if (mRenderSettings.useAdaptiveTiling)
					optimizeTileMap();
				++mIncrementalCurrentIteration;
				if (mIterationCallback)
					mIterationCallback(mIncrementalCurrentIteration - 1);
				mThreadsWaitingForIteration = 0;
				lk.unlock();

				mIterationCondition.notify_all();
			} else { // Wait for the last thread
				const uint32 callee_iter = mIncrementalCurrentIteration;
				mIterationCondition.wait(lk, [&] { return mShouldStop
														  || callee_iter < mIncrementalCurrentIteration; });
				lk.unlock();
			}
		}

		if (mTileMap->allFinished()) {
			breakBecauseFinished = true;
			break;
		}
	}

	if (breakBecauseFinished) {
		mThreadsWaitingForIteration = 0;
		mIterationCondition.notify_all();
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
	status.setField("global.depth_count", s.depthCount());
	status.setField("global.iteration_count", (uint64)mIncrementalCurrentIteration.load());

	return status;
}

} // namespace PR
