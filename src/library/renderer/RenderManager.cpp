#include "RenderManager.h"

#include "camera/CameraManager.h"
#include "entity/EntityManager.h"
#include "infinitelight/InfiniteLightManager.h"
#include "light/LightManager.h"
#include "material/MaterialManager.h"
#include "plugin/PluginManager.h"
#include "registry/Registry.h"
#include "renderer/RenderFactory.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
RenderManager::RenderManager(const std::string& workingDir)
	: mWorkingDir(workingDir)
	, mRegistry(std::make_shared<Registry>())
	, mSpectrumDescriptor(SpectrumDescriptor::createStandardSpectral())
	, mPluginManager(std::make_shared<PluginManager>())
	, mMaterialManager(std::make_shared<MaterialManager>())
	, mEntityManager(std::make_shared<EntityManager>())
	, mCameraManager(std::make_shared<CameraManager>())
	, mLightManager(std::make_shared<LightManager>())
	, mInfiniteLightManager(std::make_shared<InfiniteLightManager>())
{
}

RenderManager::~RenderManager()
{
}

std::shared_ptr<Scene> RenderManager::createScene() const
{
	return std::make_shared<Scene>(this);
}

std::shared_ptr<RenderFactory> RenderManager::createRenderFactory() const
{
	return std::make_shared<RenderFactory>(this);
}
} // namespace PR
