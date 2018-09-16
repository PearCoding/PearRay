#include "RenderContext.h"
#include "OutputMap.h"
#include "RenderManager.h"
#include "RenderSession.h"
#include "RenderThread.h"
#include "RenderTile.h"
#include "RenderTileMap.h"

#include "camera/ICamera.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "ray/Ray.h"
#include "scene/Scene.h"

#include "infinitelight/IInfiniteLight.h"

#include "integrator/IIntegrator.h"

#include "shader/ShadingPoint.h"

#include "Logger.h"
#include "math/MSI.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include "material/IMaterial.h"
#include "material/MaterialManager.h"

namespace PR {
RenderContext::RenderContext(uint32 index, uint32 ox, uint32 oy,
							 uint32 w, uint32 h,
							 const std::shared_ptr<Scene>& scene,
							 const RenderManager* manager)
	: mIndex(index)
	, mOffsetX(ox)
	, mOffsetY(oy)
	, mWidth(w)
	, mHeight(h)
	, mRenderManager(manager)
	, mScene(scene)
	, mOutputMap()
	, mTileMap()
	, mIncrementalCurrentSample(0)
	, mRenderSettings(mRenderManager->registry())
	, mIntegrator(nullptr)
	, mShouldStop(false)
{
	PR_ASSERT(mRenderManager, "Render manager can not be NULL!");
	reset();

	mOutputMap = std::make_unique<OutputMap>(this);
}

RenderContext::~RenderContext()
{
	reset();
}

void RenderContext::reset()
{
	mShouldStop				  = false;
	mThreadsWaitingForPass	= 0;
	mCurrentPass			  = 0;
	mIncrementalCurrentSample = 0;

	mIntegrator.reset();
	
	for (RenderThread* thread : mThreads)
		delete thread;

	mThreads.clear();
	mLights.clear();
}

void RenderContext::start(uint32 tcx, uint32 tcy, int32 threads)
{
	reset();

	PR_ASSERT(mOutputMap, "Output Map must be already created!");

	/* Setup entities */
	for (auto entity : mRenderManager->entityManager()->getAll()) {
		if (entity->isLight())
			mLights.push_back(entity.get());
	}

	/* Setup integrators */
	//mIntegrator = Integrator::create(this, mRenderSettings.integratorMode());

	PR_ASSERT(mIntegrator, "Integrator should be set after selection");

	mMaxRayDepth	 = mRenderSettings.maxRayDepth();
	mSamplesPerPixel = mRenderSettings.samplesPerPixel();

	PR_LOG(L_INFO) << "Rendering with: " << std::endl;
	PR_LOG(L_INFO) << "  AA Samples: " << mRenderSettings.aaSampleCount() << std::endl;
	PR_LOG(L_INFO) << "  Lens Samples: " << mRenderSettings.lensSampleCount() << std::endl;
	PR_LOG(L_INFO) << "  Time Samples: " << mRenderSettings.timeSampleCount() << std::endl;
	PR_LOG(L_INFO) << "  Spectral Samples: " << mRenderSettings.spectralSampleCount() << std::endl;
	PR_LOG(L_INFO) << "  Full Samples: " << mSamplesPerPixel << std::endl;

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

	/* Setup scene */
	mScene->setup(this);

	// Calculate tile sizes, etc.
	uint32 ptcx = std::max<uint32>(1, tcx);
	uint32 ptcy = std::max<uint32>(1, tcy);
	uint32 ptcw = std::max<uint32>(1, std::floor(mWidth / static_cast<float>(ptcx)));
	uint32 ptch = std::max<uint32>(1, std::floor(mHeight / static_cast<float>(ptcy)));
	ptcx		= std::ceil(mWidth / static_cast<float>(ptcw));
	ptcy		= std::ceil(mHeight / static_cast<float>(ptch));

	mTileMap = std::make_unique<RenderTileMap>(ptcx, ptcy, ptcw, ptch);
	mTileMap->init(*this, mRenderSettings.tileMode());

	// Init modules
	mIntegrator->init();
	
	mOutputMap->clear();

	// Start
	mIntegrator->onStart(); // TODO: onEnd
	if (mIntegrator->needNextPass(0)) {
		bool clear; // Doesn't matter, as it is already clean.
		mIntegrator->onNextPass(0, clear);
	}

	PR_LOG(L_INFO) << "Rendering with " << threadCount << " threads." << std::endl;
	PR_LOG(L_INFO) << "Starting threads." << std::endl;
	for (RenderThread* thread : mThreads)
		thread->start();
}

uint32 RenderContext::tileCount() const
{
	return mTileMap->tileCount();
}

std::list<RenderTile*> RenderContext::currentTiles() const
{
	std::list<RenderTile*> list;
	for (RenderThread* thread : mThreads) {
		RenderTile* tile = thread->currentTile();
		if (tile)
			list.push_back(tile);
	}
	return list;
}

void RenderContext::traceRays(RayStream& rays, HitStream& hits)
{
	mScene->traceRays(rays, hits);
}

void RenderContext::waitForNextPass()
{
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
	for (RenderThread* thread : mThreads) {
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
	std::lock_guard<std::mutex> guard(mTileMutex);

	RenderTile* tile = nullptr;

	// Try till we find a tile or all samples are already rendered
	while (tile == nullptr && mIncrementalCurrentSample <= mSamplesPerPixel) {
		tile = mTileMap->getNextTile(std::min(mIncrementalCurrentSample, mSamplesPerPixel));
		if (tile == nullptr) {
			mIncrementalCurrentSample++;
		}
	}

	return tile;
}

RenderStatistics RenderContext::statistics() const
{
	return mTileMap->statistics();
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
	mTileMap->reset();

	bool clear = false;
	mIntegrator->onNextPass(mCurrentPass + 1, clear);

	mIncrementalCurrentSample = 0;

	if (clear)
		mOutputMap->clear();
}
} // namespace PR
