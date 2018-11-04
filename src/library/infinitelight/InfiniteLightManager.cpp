#include "InfiniteLightManager.h"
#include "IInfiniteLightFactory.h"
#include "plugin/PluginManager.h"
#include "renderer/RenderManager.h"

#include "Logger.h"

namespace PR {
InfiniteLightManager::InfiniteLightManager()
{
}

InfiniteLightManager::~InfiniteLightManager()
{
}

bool InfiniteLightManager::loadFactory(const RenderManager& mng,
									   const std::string& base, const std::string& name)
{
	auto plugin = mng.pluginManager()->load(base + "pr_pl_" + name, *mng.registry());
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load infinite light plugin " << name << " with base path " << base << std::endl;
		return false;
	}

	if (plugin->type() != PT_INFINITELIGHT) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a infinite light plugin!" << std::endl;
		return false;
	}

	auto matF = std::dynamic_pointer_cast<IInfiniteLightFactory>(plugin);
	if (!matF) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a infinite light plugin even when it says it is!" << std::endl;
		return false;
	}

	auto names = matF->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Infinite light with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = matF;
	}
	return true;
}

} // namespace PR
