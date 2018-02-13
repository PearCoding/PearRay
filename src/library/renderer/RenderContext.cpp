#include "RenderContext.h"
#include "OutputMap.h"
#include "RenderSession.h"
#include "RenderThread.h"
#include "RenderTile.h"
#include "RenderTileMap.h"

#include "camera/Camera.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "scene/Scene.h"

#include "integrator/BiDirectIntegrator.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/PPMIntegrator.h"

#include "light/IInfiniteLight.h"

#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

#include "Logger.h"
#include "math/MSI.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include "material/Material.h"

#ifndef PR_NO_GPU
#include "gpu/GPU.h"
#endif

namespace PR {
RenderContext::RenderContext(uint32 index, uint32 ox, uint32 oy, uint32 w, uint32 h, uint32 fw, uint32 fh,
							 const SpectrumDescriptor& specdesc, const Scene& scene, const std::string& workingDir, GPU* gpu, const RenderSettings& settings)
	: mIndex(index)
	, mOffsetX(ox)
	, mOffsetY(oy)
	, mWidth(w)
	, mHeight(h)
	, mFullWidth(fw)
	, mFullHeight(fh)
	, mWorkingDir(workingDir)
	, mSpectrumDescriptor(specdesc)
	, mCamera(scene.activeCamera())
	, mScene(scene)
	, mOutputMap()
	, mTileMap()
	, mIncrementalCurrentSample(0)
	, mRenderSettings(settings)
	, mGPU(gpu)
	, mIntegrator(nullptr)
	, mShouldStop(false)
{
	PR_ASSERT(mCamera, "Given camera has to be valid");

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

	if (mIntegrator) {
		delete mIntegrator;
		mIntegrator = nullptr;
	}

	for (RenderThread* thread : mThreads)
		delete thread;

	mThreads.clear();
	mLights.clear();
}

void RenderContext::start(uint32 tcx, uint32 tcy, int32 threads)
{
	reset();

	/* Setup entities */
	for (const auto& entity : mScene.renderEntities()) {
		if (entity->isLight())
			mLights.push_back(entity.get());
	}

	/* Setup integrators */
	if (mRenderSettings.debugMode() != DM_None) {
		mIntegrator = new DebugIntegrator(this);
	} else {
		if (mRenderSettings.maxLightSamples() == 0) {
			PR_LOGGER.log(L_Warning, M_Scene, "MaxLightSamples is zero: Nothing to render");
			return;
		}

		switch (mRenderSettings.integratorMode()) {
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
	PR_LOGGER.logf(L_Info, M_Scene, "  AA Samples: %i", mRenderSettings.maxAASampleCount());
	PR_LOGGER.logf(L_Info, M_Scene, "  Lens Samples: %i", mRenderSettings.maxLensSampleCount());
	PR_LOGGER.logf(L_Info, M_Scene, "  Time Samples: %i", mRenderSettings.maxTimeSampleCount());
	PR_LOGGER.logf(L_Info, M_Scene, "  Spectral Samples: %i", mRenderSettings.maxSpectralSampleCount());

	/* Setup threads */
	uint32 threadCount = Thread::hardwareThreadCount();
	if (threads < 0)
		threadCount = std::max(1, (int32)threadCount + threads);
	else if (threads > 0)
		threadCount = threads;

	for (uint32 i = 0; i < threadCount; ++i) {
		RenderThread* thread = new RenderThread(i, this);
		mThreads.push_back(thread);
	}

	// Calculate tile sizes, etc.
	mTileMap = std::make_unique<RenderTileMap>(
		std::max<uint32>(1, tcx),
		std::max<uint32>(1, tcy),
		(uint32)std::ceil(mWidth / (float)std::max<uint32>(1, tcx)),
		(uint32)std::ceil(mHeight / (float)std::max<uint32>(1, tcy)));
	mTileMap->init(*this, mRenderSettings.tileMode());

	// Init modules
	mIntegrator->init();
	mOutputMap->init();

	// Start
	mIntegrator->onStart(); // TODO: onEnd?
	if (mIntegrator->needNextPass(0)) {
		bool clear; // Doesn't matter, as it is already clean.
		mIntegrator->onNextPass(0, clear);
	}

	PR_LOGGER.logf(L_Info, M_Scene, "Rendering with %d threads.", threadCount);
	PR_LOGGER.log(L_Info, M_Scene, "Starting threads.");
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

RenderEntity* RenderContext::shoot(const Ray& ray, ShaderClosure& sc, const RenderSession& session)
{
	if (ray.depth() < mRenderSettings.maxRayDepth()) {
		SceneCollision c = mScene.checkCollision(ray);
		sc				 = c.Point;

		sc.Flags  = 0;
		sc.NgdotV = ray.direction().dot(sc.Ng);
		sc.N	  = Reflection::faceforward(sc.NgdotV, sc.Ng);
		sc.Flags |= Reflection::is_inside(sc.NgdotV) ? SCF_Inside : 0;
		sc.NdotV		   = -std::abs(sc.NgdotV);
		sc.V			   = ray.direction();
		sc.T			   = ray.time();
		sc.WavelengthIndex = ray.wavelength();
		sc.Depth2		   = (ray.origin() - sc.P).squaredNorm();

		if (c.Entity)
			sc.EntityID = c.Entity->id();

		if (sc.Flags & SCF_Inside) {
			sc.Nx = -sc.Nx;
			//sc.Ny = -sc.Ny;
		}

		session.tile()->statistics().incRayCount();
		if (c.Entity)
			session.tile()->statistics().incEntityHitCount();

		return c.Entity;
	} else {
		return nullptr;
	}
}

bool RenderContext::shootForDetection(const Ray& ray, const RenderSession& session)
{
	if (ray.depth() < mRenderSettings.maxRayDepth()) {
		SceneCollision c = mScene.checkCollisionSimple(ray);

		session.tile()->statistics().incRayCount();
		if (c.Successful)
			session.tile()->statistics().incEntityHitCount();

		return c.Successful;
	} else {
		return false;
	}
}

RenderEntity* RenderContext::shootWithEmission(Spectrum& appliedSpec, const Ray& ray,
											   ShaderClosure& sc, const RenderSession& session)
{
	if (ray.depth() >= mRenderSettings.maxRayDepth())
		return nullptr;

	RenderEntity* entity = shoot(ray, sc, session);
	if (entity) {
		if (sc.Material)
			sc.Material->evalEmission(appliedSpec, sc, session);
		else
			appliedSpec.clear();
	} else {
		appliedSpec.clear();

		for (const auto& e : mScene.infiniteLights())
			e->apply(appliedSpec, ray.direction(), session);

		session.tile()->statistics().incBackgroundHitCount();
	}

	return entity;
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

	const auto maxCameraSamples = mRenderSettings.maxCameraSampleCount();
	RenderTile* tile			= nullptr;

	// Try till we find a tile or all samples are already rendered
	while (tile == nullptr && mIncrementalCurrentSample <= maxCameraSamples) {
		tile = mTileMap->getNextTile(std::min(mIncrementalCurrentSample, maxCameraSamples));
		if (tile == nullptr && mIncrementalCurrentSample < maxCameraSamples) {
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

	if (clear)
		mOutputMap->clear();
}
}
