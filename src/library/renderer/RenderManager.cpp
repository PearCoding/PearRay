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
	static const boost::regex e("(lib)?pr_pl_(\\w+)_(\\w+)_d");
#else
	static const boost::regex e("(lib)?pr_pl_(\\w+)_(\\w+)");
#endif

	for (auto& entry :
		 boost::make_iterator_range(boost::filesystem::directory_iterator(basedir), {})) {
		if (!boost::filesystem::is_regular_file(entry))
			continue;

		const std::string filename = entry.path().stem().string();

		boost::smatch what;
		if (boost::regex_match(filename, what, e)) {
			const std::string type = what[2];
			const std::string name = type + "_" + what[3];

			if (type == "int") {
				mIntegratorManager->loadFactory(*this, basedir, name,
												"integrator", PT_INTEGRATOR);
			} else if (type == "ent") {
				mEntityManager->loadFactory(*this, basedir, name,
											"entity", PT_ENTITY);
			} else if (type == "cam") {
				mCameraManager->loadFactory(*this, basedir, name,
											"camera", PT_CAMERA);
			} else if (type == "emi") {
				mEmissionManager->loadFactory(*this, basedir, name,
											  "emission", PT_EMISSION);
			} else if (type == "mat") {
				mMaterialManager->loadFactory(*this, basedir, name,
											  "material", PT_MATERIAL);
			} else if (type == "lig") {
				mInfiniteLightManager->loadFactory(*this, basedir, name,
												   "infinitelight", PT_INFINITELIGHT);
			}
		}
	}
}
} // namespace PR
