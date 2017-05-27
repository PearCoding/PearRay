#include "RenderFactory.h"
#include "Logger.h"
#include "RenderContext.h"
#include "scene/Scene.h"

#ifndef PR_NO_GPU
#include "gpu/GPU.h"
#endif

namespace PR {
RenderFactory::RenderFactory(uint32 w, uint32 h, const Scene& scene,
							 const std::string& workingDir, bool useGPU)
	: mFullWidth(w)
	, mFullHeight(h)
	, mWorkingDir(workingDir)
	, mScene(scene)
	, mGPU(nullptr)
{
// Setup GPU
#ifndef PR_NO_GPU
	if (useGPU) {
		mGPU = new GPU();
		if (!mGPU->init("")) {
			delete mGPU;
			mGPU = nullptr;
		}
	}
#endif
}

RenderFactory::~RenderFactory()
{
#ifndef PR_NO_GPU
	if (mGPU) {
		delete mGPU;
	}
#endif
}

uint32 RenderFactory::cropWidth() const
{
	return std::ceil((mRenderSettings.cropMaxX() - mRenderSettings.cropMinX()) * mFullWidth);
}

uint32 RenderFactory::cropHeight() const
{
	return std::ceil((mRenderSettings.cropMaxY() - mRenderSettings.cropMinY()) * mFullHeight);
}

uint32 RenderFactory::cropOffsetX() const
{
	return std::ceil(mRenderSettings.cropMinX() * mFullWidth);
}

uint32 RenderFactory::cropOffsetY() const
{
	return std::ceil(mRenderSettings.cropMinY() * mFullHeight);
}

std::shared_ptr<RenderContext> RenderFactory::create(uint32 index, uint32 itx, uint32 ity) const
{
	if (!mScene.activeCamera()) {
		PR_LOGGER.log(L_Error, M_Scene, "No active camera selected");
		return std::shared_ptr<RenderContext>();
	}

	PR_ASSERT(itx > 0, "Image tile count x has to be greater 0");
	PR_ASSERT(ity > 0, "Image tile count y has to be greater 0");
	PR_ASSERT(index < itx * ity, "Index has to be in bounds");

	uint32 itw = (uint32)std::ceil(cropWidth() / (float)itx);
	uint32 ith = (uint32)std::ceil(cropHeight() / (float)ity);
	uint32 ix  = index % itx;
	uint32 iy  = index / itx;
	uint32 x   = ix * itw;
	uint32 y   = iy * ith;

	return std::make_shared<RenderContext>(index, x + cropOffsetX(), y + cropOffsetY(),
										   itw, ith, mFullWidth, mFullHeight,
										   mScene, mWorkingDir, mGPU, mRenderSettings);
}
}
