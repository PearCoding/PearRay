#include "RenderFactory.h"
#include "Logger.h"
#include "RenderContext.h"
#include "RenderManager.h"
#include "camera/CameraManager.h"
#include "material/MaterialManager.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderFactory::RenderFactory(const RenderManager* renderManager)
	: mRenderManager(renderManager)
{
}

RenderFactory::~RenderFactory()
{
}

std::shared_ptr<RenderContext> RenderFactory::create(uint32 index, uint32 itx, uint32 ity) const
{
	if (!mRenderManager->cameraManager()->hasActiveCamera()) {
		PR_LOG(L_ERROR) << "No active camera selected!" << std::endl;
		return std::shared_ptr<RenderContext>();
	}

	PR_ASSERT(itx > 0, "Image tile count x has to be greater 0");
	PR_ASSERT(ity > 0, "Image tile count y has to be greater 0");
	PR_ASSERT(index < itx * ity, "Index has to be in bounds");

	RenderSettings settings(mRenderManager->registry());

	uint32 itw = std::ceil(settings.cropWidth() / static_cast<float>(itx));
	uint32 ith = std::ceil(settings.cropHeight() / static_cast<float>(ity));
	uint32 ix  = index % itx;
	uint32 iy  = index / itx;
	uint32 x   = ix * itw;
	uint32 y   = iy * ith;

	return std::make_shared<RenderContext>(index,
										   x + settings.cropOffsetX(), y + settings.cropOffsetY(),
										   itw, ith,
										   mRenderManager->createScene(),
										   mRenderManager);
}
} // namespace PR
