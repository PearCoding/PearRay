#include "CameraManager.h"
#include "ICameraFactory.h"
#include "plugin/PluginLoader.h"

#include "Logger.h"

namespace PR {
CameraManager::CameraManager()
	: AbstractManager()
	, mActiveCamera(0)
{
}

CameraManager::~CameraManager()
{
	/* Remark: ICameraFactory pointer are not freed! The plugin itself does the work */
}

void CameraManager::loadFactory(const Registry& reg,
								const std::string& base, const std::string& name)
{
	IPlugin* plugin = PluginLoader::load(base + "pr_pl_" + name, reg);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load camera plugin " << name << " with base path " << base << std::endl;
		return;
	}

	if (plugin->type() != PT_CAMERA) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a camera plugin!" << std::endl;
		return;
	}

	ICameraFactory* fac = dynamic_cast<ICameraFactory*>(plugin);
	if (!fac) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a camera plugin even when it says it is!" << std::endl;
		return;
	}

	auto names = fac->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Camera with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = fac;
	}
}

} // namespace PR
