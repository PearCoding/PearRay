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
	Point1i index, const Size2i& imageTileSize) const
{
	if (!integrator) {
		PR_LOG(L_ERROR) << "No integrator given!" << std::endl;
		return nullptr;
	}

	PR_ASSERT(imageTileSize.isValid(), "Image tile size has to be valid");
	PR_ASSERT(index < imageTileSize.area(), "Index has to be in bounds");

	Size1i itw = static_cast<Size1i>(std::ceil(mSettings.cropWidth() / static_cast<float>(imageTileSize.Width)));
	Size1i ith = static_cast<Size1i>(std::ceil(mSettings.cropHeight() / static_cast<float>(imageTileSize.Height)));
	Point1i ix = index % imageTileSize.Width;
	Point1i iy = index / imageTileSize.Width;
	Point1i x  = ix * itw;
	Point1i y  = iy * ith;

	return std::make_shared<RenderContext>(index,
										   Point2i(x + mSettings.cropOffsetX(),
													y + mSettings.cropOffsetY()),
										   Size2i(itw, ith),
										   integrator,
										   mScene,
										   mSpectrumDescriptor,
										   mSettings);
}
} // namespace PR
