#include "RenderManager.h"

#include "camera/CameraManager.h"
#include "emission/EmissionManager.h"
#include "entity/EntityManager.h"
#include "infinitelight/InfiniteLightManager.h"
#include "integrator/IntegratorManager.h"
#include "material/MaterialManager.h"
#include "plugin/PluginManager.h"
#include "registry/Registry.h"
#include "renderer/RenderFactory.h"
#include "scene/Scene.h"
#include "spectral/SpectrumDescriptor.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace PR {
RenderManager::RenderManager(const std::string& workingDir)
	: mWorkingDir(workingDir)
	, mRegistry(std::make_shared<Registry>())
	, mSpectrumDescriptor(SpectrumDescriptor::createStandardSpectral())
	, mPluginManager(std::make_shared<PluginManager>())
	, mMaterialManager(std::make_shared<MaterialManager>())
	, mEntityManager(std::make_shared<EntityManager>())
	, mCameraManager(std::make_shared<CameraManager>())
	, mEmissionManager(std::make_shared<EmissionManager>())
	, mInfiniteLightManager(std::make_shared<InfiniteLightManager>())
	, mIntegratorManager(std::make_shared<IntegratorManager>())
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

void RenderManager::loadPlugins(const std::string& basedir)
{
#ifdef PR_DEBUG
	static const boost::regex e("(lib)?pr_pl_([\\w_]+)_d");
#else
	static const boost::regex e("(lib)?pr_pl_([\\w_]+)");
#endif

	for (auto& entry :
		 boost::make_iterator_range(boost::filesystem::directory_iterator(basedir), {})) {
		if (!boost::filesystem::is_regular_file(entry))
			continue;

		const std::string filename = entry.path().stem().string();

		boost::smatch what;
		if (boost::regex_match(filename, what, e)) {
			loadOnePlugin(entry.path().string());
		}
	}
}

void RenderManager::loadOnePlugin(const std::string& name)
{
	auto plugin = mPluginManager->load(name, *mRegistry);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Could not load plugin " << name << std::endl;
		return;
	}

	switch (plugin->type()) {
	case PT_INTEGRATOR:
		mIntegratorManager->addFactory(std::dynamic_pointer_cast<IIntegratorFactory>(plugin));
		break;
	case PT_CAMERA:
		mCameraManager->addFactory(std::dynamic_pointer_cast<ICameraFactory>(plugin));
		break;
	case PT_MATERIAL:
		mMaterialManager->addFactory(std::dynamic_pointer_cast<IMaterialFactory>(plugin));
		break;
	case PT_EMISSION:
		mEmissionManager->addFactory(std::dynamic_pointer_cast<IEmissionFactory>(plugin));
		break;
	case PT_INFINITELIGHT:
		mInfiniteLightManager->addFactory(std::dynamic_pointer_cast<IInfiniteLightFactory>(plugin));
		break;
	case PT_ENTITY:
		mEntityManager->addFactory(std::dynamic_pointer_cast<IEntityFactory>(plugin));
		break;
	default:
		PR_LOG(L_ERROR) << "Plugin " << name << " has unknown plugin type." << std::endl;
		return;
	}
}
} // namespace PR
