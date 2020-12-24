#include "RenderContext.h"
#include "Logger.h"
#include "Platform.h"
#include "Profiler.h"
#include "RenderThread.h"
#include "RenderTile.h"
#include "RenderTileMap.h"
#include "RenderTileSession.h"
#include "camera/ICamera.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "light/LightSampler.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Scattering.h"
#include "output/OutputSystem.h"
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
	, mScene(scene)
	, mOutputSystem(std::make_unique<OutputSystem>(viewSize))
	, mTileMap()
	, mThreadsWaitingForIteration(0)
	, mIncrementalCurrentIteration(0)
	, mRenderSettings(settings)
	, mIntegrator(integrator)
	, mIntegratorPassCount(1)
	, mShouldStop(false)

	, mShouldSoftStop(false)
{
	PR_ASSERT(mIntegrator, "Integrator can not be NULL!");
	PR_ASSERT(mScene, "Scene can not be NULL!");
	reset();
}

RenderContext::~RenderContext()
{
	reset();
}

void RenderContext::reset()
{
	PR_PROFILE_THIS;

	mThreads.clear();

	mShouldStop					 = false;
	mShouldSoftStop				 = false;
	mOutputClearRequest			 = false;
	mThreadsWaitingForIteration	 = 0;
	mIncrementalCurrentIteration = 0;
}

void RenderContext::start(uint32 rtx, uint32 rty, int32 threads)
{
	PR_PROFILE_THIS;

	setupFloatingPointEnvironment();

	reset();

	/* Setup threads */
	uint32 threadCount = Thread::hardwareThreadCount();
	if (threads < 0)
		threadCount = std::max(1, static_cast<int32>(threadCount) + threads);
	else if (threads > 0)
		threadCount = threads;

	for (uint32 i = 0; i < threadCount; ++i)
		mThreads.emplace_back(std::make_unique<RenderThread>(i, this));

	// Setup light sampler
	mLightSampler = std::make_shared<LightSampler>(mScene.get());

	// Setup tile map
	mTileMap = std::make_unique<RenderTileMap>();
	mTileMap->init(this, rtx, rty, mRenderSettings.tileMode);

	// Init modules
	mIntegrator->onInit(this);
	mOutputSystem->clear();

	mIntegratorPassCount = mIntegrator->configuration().PassCount;

	// Call all interested objects after thread count is fixed
	mScene->beforeRender(this);

	// Get other informations
	PR_LOG(L_INFO) << "Rendering with:" << std::endl
				   << "  Threads:                " << threadCount << std::endl
				   << "  Passes:                 " << mIntegratorPassCount << std::endl
				   << "  Tiles:                  " << rtx << " x " << rty << std::endl
				   << "  Entities:               " << mScene->entityCount() << std::endl
				   << "  Lights:                 " << mLightSampler->emissiveEntityCount() << std::endl
				   << "  Materials:              " << mScene->materialCount() << std::endl
				   << "  Emissions:              " << mScene->emissionCount() << std::endl
				   << "  InfLights:              " << mScene->infiniteLightCount() << std::endl
				   << "  Nodes:                  " << mScene->nodeCount() << std::endl
				   << "  Emissive Surface Area:  " << mLightSampler->emissiveSurfaceArea() << std::endl
				   << "  Emissive Surface Power: " << mLightSampler->emissiveSurfacePower() << std::endl
				   << "  Emissive Power:         " << mLightSampler->emissivePower() << std::endl
				   << "  Scene Extent:           " << mScene->boundingBox().width() << " x " << mScene->boundingBox().height() << " x " << mScene->boundingBox().depth() << std::endl
				   << "  Scene Origin Radius:    " << mScene->boundingSphere().radius() << std::endl
				   << "  Spectral Domain:        [" << mRenderSettings.spectralStart << ", " << mRenderSettings.spectralEnd << "]" << std::endl
				   << "  Adaptive Tiling:        " << (mRenderSettings.useAdaptiveTiling ? "true" : "false") << std::endl;

	// Start
	mIntegrator->onStart();
	for (const auto& clb : mIterationCallbacks)
		clb(RenderIteration{ 0, 0 });
	PR_LOG(L_INFO) << "Starting threads." << std::endl;
	for (const auto& thread : mThreads)
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

	for (const auto& thread : mThreads) {
		RenderTile* tile = thread->currentTile();
		if (tile)
			list.push_back(Rect2i(tile->start(), tile->viewSize()));
	}
	return list;
}

bool RenderContext::isFinished() const
{
	PR_PROFILE_THIS;

	for (const auto& thread : mThreads) {
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
	for (const auto& thread : mThreads)
		thread->join();
}

void RenderContext::requestStop()
{
	requestInternalStop();

	// Make sure threads are not inside conditions
	mIterationCondition.notify_all();
}

void RenderContext::requestSoftStop()
{
	mShouldSoftStop = true;
}

void RenderContext::requestInternalStop()
{
	mShouldStop = true;

	// Request all threads to stop
	for (const auto& thread : mThreads)
		thread->requestStop();
}

void RenderContext::stop()
{
	PR_PROFILE_THIS;

	requestStop();

	// Wait for all threads to stop
	for (const auto& thread : mThreads)
		thread->join();
}

RenderTile* RenderContext::getNextTile()
{
	PR_PROFILE_THIS;

	const size_t threads = threadCount();

	RenderTile* tile = nullptr;

	// Try till we find a tile to render
	while (tile == nullptr) {
		tile = mTileMap->getNextTile();
		if (tile == nullptr) { // If no tile on this iteration was found sync all threads
			std::unique_lock<std::mutex> lk(mIterationMutex);
			++mThreadsWaitingForIteration;

			if (mThreadsWaitingForIteration == threads) { // The last thread arriving here has the honor to call optional callbacks and initiate the next iteration
				handleNextIteration();
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

		if (mShouldStop || mTileMap->allFinished()) {
			mThreadsWaitingForIteration = 0;
			mIterationCondition.notify_all();
			return nullptr;
		}
	}

	return tile;
}

void RenderContext::handleNextIteration()
{
	if (mShouldSoftStop)
		requestInternalStop();

	mTileMap->makeAllIdle();

	if (mOutputClearRequest.exchange(false)) {
		PR_LOG(L_DEBUG) << "Clearing output buffer" << std::endl;
		mOutputSystem->clear(true);
	}

	++mIncrementalCurrentIteration;
	const RenderIteration iter = currentIteration();

	if (iter.Pass == 0 && mRenderSettings.useAdaptiveTiling) {
		PR_LOG(L_DEBUG) << "Optimizing tile map" << std::endl;
		optimizeTileMap();
	}

	for (const auto& clb : mIterationCallbacks)
		clb(iter);
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

	RenderStatus status = mIntegrator->status();

	const RenderTileStatistics s = statistics();
	const RenderIteration iter	 = currentIteration();

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
	status.setField("global.camera_depth_count", s.cameraDepthCount());
	status.setField("global.light_depth_count", s.lightDepthCount());
	status.setField("global.iteration_count", (uint64)iter.Iteration);
	status.setField("global.pass_count", (uint64)iter.Pass);

	return status;
}

} // namespace PR
