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

namespace PR {
RenderContext::RenderContext(uint32 index, uint32 ox, uint32 oy,
							 uint32 w, uint32 h,
							 const std::shared_ptr<IIntegrator>& integrator,
							 const std::shared_ptr<Scene>& scene,
							 const std::shared_ptr<SpectrumDescriptor>& specDesc,
							 const RenderSettings& settings)
	: mIndex(index)
	, mOffsetX(ox)
	, mOffsetY(oy)
	, mWidth(w)
	, mHeight(h)
	, mScene(scene)
	, mSpectrumDescriptor(specDesc)
	, mOutputMap()
	, mEmissiveSurfaceArea(0.0f)
	, mTileMap()
	, mIncrementalCurrentSample(0)
	, mRenderSettings(settings)
	, mIntegrator(integrator)
	, mShouldStop(false)
{
	PR_ASSERT(mIntegrator, "Integrator can not be NULL!");
	PR_ASSERT(mScene, "Scene can not be NULL!");
	PR_ASSERT(mSpectrumDescriptor, "Spectrum Descriptor can not be NULL!");
	reset();

	mOutputMap = std::make_unique<OutputBuffer>(this);
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

	mShouldStop				  = false;
	mThreadsWaitingForPass	= 0;
	mCurrentPass			  = 0;
	mIncrementalCurrentSample = 0;

	mThreads.clear();
	mLights.clear();
}

void RenderContext::start(uint32 tcx, uint32 tcy, int32 threads)
{
	PR_PROFILE_THIS;

	reset();

	PR_ASSERT(mOutputMap, "Output Map must be already created!");

	/* Setup entities */
	mEmissiveSurfaceArea = 0.0f;
	for (auto entity : mScene->entities()) {
		if (entity->isLight()) {
			mEmissiveSurfaceArea += entity->surfaceArea();
			mLights.push_back(entity);
		}
	}

	// Get other informations
	mMaxRayDepth	 = mRenderSettings.maxRayDepth;
	mSamplesPerPixel = mRenderSettings.samplesPerPixel();

	PR_LOG(L_INFO) << "Rendering with: " << std::endl;
	PR_LOG(L_INFO) << "  Lights: " << mLights.size() << std::endl;
	PR_LOG(L_INFO) << "  AA Samples: " << mRenderSettings.aaSampleCount << std::endl;
	PR_LOG(L_INFO) << "  Lens Samples: " << mRenderSettings.lensSampleCount << std::endl;
	PR_LOG(L_INFO) << "  Time Samples: " << mRenderSettings.timeSampleCount << std::endl;
	PR_LOG(L_INFO) << "  Full Samples: " << mSamplesPerPixel << std::endl;
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

	// Calculate tile sizes, etc.
	uint32 ptcx = std::max<uint32>(1, tcx);
	uint32 ptcy = std::max<uint32>(1, tcy);
	uint32 ptcw = std::max<uint32>(1, std::floor(mWidth / static_cast<float>(ptcx)));
	uint32 ptch = std::max<uint32>(1, std::floor(mHeight / static_cast<float>(ptcy)));
	ptcx		= std::ceil(mWidth / static_cast<float>(ptcw));
	ptcy		= std::ceil(mHeight / static_cast<float>(ptch));

	mTileMap = std::make_unique<RenderTileMap>(ptcx, ptcy, ptcw, ptch);
	mTileMap->init(*this, mRenderSettings.tileMode);

	// Init modules
	mIntegrator->onInit(this);

	mOutputMap->clear();

	// Start
	mIntegrator->onStart();
	if (mIntegrator->needNextPass(0)) {
		bool clear; // Doesn't matter, as it is already clean.
		mIntegrator->onNextPass(0, clear);
	}

	PR_LOG(L_INFO) << "Rendering with " << threadCount << " threads." << std::endl;
	PR_LOG(L_INFO) << "Starting threads." << std::endl;
	for (RenderThread* thread : mThreads)
		thread->start();
}

void RenderContext::notifyEnd()
{
	PR_PROFILE_THIS;

	if (isFinished() && mIntegrator)
		mIntegrator->onEnd();
}

uint32 RenderContext::tileCount() const
{
	return mTileMap->tileCount();
}

std::list<RenderTile*> RenderContext::currentTiles() const
{
	PR_PROFILE_THIS;

	std::list<RenderTile*> list;
	for (RenderThread* thread : mThreads) {
		RenderTile* tile = thread->currentTile();
		if (tile)
			list.push_back(tile);
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
}

RenderTile* RenderContext::getNextTile()
{
	PR_PROFILE_THIS;

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

RenderTileStatistics RenderContext::statistics() const
{
	return mTileMap->statistics();
}

RenderStatus RenderContext::status() const
{
	PR_PROFILE_THIS;

	RenderTileStatistics s = statistics();

	RenderStatus status = mIntegrator->status();
	status.setField("global.ray_count", s.rayCount());
	status.setField("global.pixel_sample_count", s.pixelSampleCount());
	status.setField("global.entity_hit_count", s.entityHitCount());
	status.setField("global.background_hit_count", s.backgroundHitCount());
	return status;
}

void RenderContext::onNextPass()
{
	PR_PROFILE_THIS;

	mTileMap->reset();

	bool clear = false;
	mIntegrator->onNextPass(mCurrentPass + 1, clear);

	mIncrementalCurrentSample = 0;

	if (clear)
		mOutputMap->clear();
}
} // namespace PR
