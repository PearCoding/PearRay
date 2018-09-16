#include "InfiniteLightManager.h"
#include "IInfiniteLightFactory.h"
#include "plugin/PluginLoader.h"

#include "Logger.h"

namespace PR {
InfiniteLightManager::InfiniteLightManager()
{
}

InfiniteLightManager::~InfiniteLightManager()
{
	/* Remark: IInfiniteLightFactory pointer are not freed! The plugin itself does the work */
}

void InfiniteLightManager::loadFactory(const Registry& reg,
									   const std::string& base, const std::string& name)
{
	IPlugin* plugin = PluginLoader::load(base + "pr_pl_" + name, reg);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load infinite light plugin " << name << " with base path " << base << std::endl;
		return;
	}

	if (plugin->type() != PT_INFINITELIGHT) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a infinite light plugin!" << std::endl;
		return;
	}

	IInfiniteLightFactory* matF = dynamic_cast<IInfiniteLightFactory*>(plugin);
	if (!matF) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a infinite light plugin even when it says it is!" << std::endl;
		return;
	}

	auto names = matF->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Infinite light with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = matF;
	}
}

} // namespace PR
