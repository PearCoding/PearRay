#include "MaterialManager.h"
#include "IMaterialFactory.h"
#include "plugin/PluginLoader.h"

#include "Logger.h"

namespace PR {
MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
	/* Remark: IMaterialFactory pointer are not freed! The plugin itself does the work */
}

void MaterialManager::loadFactory(const Registry& reg,
								  const std::string& base, const std::string& name)
{
	IPlugin* plugin = PluginLoader::load(base + "pr_pl_" + name, reg);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load material plugin " << name << " with base path " << base << std::endl;
		return;
	}

	if (plugin->type() != PT_MATERIAL) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a material plugin!" << std::endl;
		return;
	}

	IMaterialFactory* matF = dynamic_cast<IMaterialFactory*>(plugin);
	if (!matF) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a material plugin even when it says it is!" << std::endl;
		return;
	}

	auto names = matF->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Material with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = matF;
	}
}

} // namespace PR
