#include "LightManager.h"
#include "ILightFactory.h"
#include "plugin/PluginLoader.h"

#include "Logger.h"

namespace PR {
LightManager::LightManager()
{
}

LightManager::~LightManager()
{
	/* Remark: ILightFactory pointer are not freed! The plugin itself does the work */
}

void LightManager::loadFactory(const Registry& reg,
							   const std::string& base, const std::string& name)
{
	IPlugin* plugin = PluginLoader::load(base + "pr_pl_" + name, reg);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load light plugin " << name << " with base path " << base << std::endl;
		return;
	}

	if (plugin->type() != PT_LIGHT) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a light plugin!" << std::endl;
		return;
	}

	ILightFactory* matF = dynamic_cast<ILightFactory*>(plugin);
	if (!matF) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a light plugin even when it says it is!" << std::endl;
		return;
	}

	auto names = matF->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Light with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = matF;
	}
}

} // namespace PR
