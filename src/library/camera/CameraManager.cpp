#include "CameraManager.h"
#include "ICameraFactory.h"
#include "plugin/PluginManager.h"
#include "renderer/RenderManager.h"

#include "Logger.h"

namespace PR {
CameraManager::CameraManager()
	: AbstractManager()
	, mActiveCamera(0)
{
}

CameraManager::~CameraManager()
{
}

bool CameraManager::loadFactory(const RenderManager& mng,
								const std::string& base, const std::string& name)
{
	auto plugin = mng.pluginManager()->load(base + "pr_pl_" + name, *mng.registry());
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load camera plugin " << name << " with base path " << base << std::endl;
		return false;
	}

	if (plugin->type() != PT_CAMERA) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a camera plugin!" << std::endl;
		return false;
	}

	auto fac = std::dynamic_pointer_cast<ICameraFactory>(plugin);
	if (!fac) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a camera plugin even when it says it is!" << std::endl;
		return false;
	}

	auto names = fac->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Camera with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = fac;
	}
	return true;
}

} // namespace PR
