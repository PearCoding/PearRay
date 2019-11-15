#include "RenderFactory.h"
#include "Logger.h"
#include "RenderContext.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderFactory::RenderFactory(const std::shared_ptr<Scene>& scene,
							 const std::shared_ptr<SpectrumDescriptor>& specDesc)
	: mSpectrumDescriptor(specDesc)
	, mScene(scene)
{
}

RenderFactory::RenderFactory(const std::shared_ptr<Scene>& scene)
	: RenderFactory(scene, SpectrumDescriptor::createDefault())
{
}

RenderFactory::~RenderFactory()
{
}

std::shared_ptr<RenderContext> RenderFactory::create(
	const std::shared_ptr<IIntegrator>& integrator,
	uint32 index, uint32 itx, uint32 ity) const
{
	if (!integrator) {
		PR_LOG(L_ERROR) << "No integrator given!" << std::endl;
		return nullptr;
	}

	PR_ASSERT(itx > 0, "Image tile count x has to be greater 0");
	PR_ASSERT(ity > 0, "Image tile count y has to be greater 0");
	PR_ASSERT(index < itx * ity, "Index has to be in bounds");

	uint32 itw = static_cast<uint32>(std::ceil(mSettings.cropWidth() / static_cast<float>(itx)));
	uint32 ith = static_cast<uint32>(std::ceil(mSettings.cropHeight() / static_cast<float>(ity)));
	uint32 ix  = index % itx;
	uint32 iy  = index / itx;
	uint32 x   = ix * itw;
	uint32 y   = iy * ith;

	return std::make_shared<RenderContext>(index,
										   x + mSettings.cropOffsetX(),
										   y + mSettings.cropOffsetY(),
										   itw, ith,
										   integrator,
										   mScene,
										   mSpectrumDescriptor,
										   mSettings);
}
} // namespace PR
