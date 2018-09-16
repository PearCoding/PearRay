#include "EntityManager.h"
#include "IEntity.h"
#include "IEntityFactory.h"
#include "plugin/PluginLoader.h"

#include "Logger.h"

namespace PR {
EntityManager::EntityManager()
	: AbstractManager()
{
}

EntityManager::~EntityManager()
{
	/* Remark: IEntityFactory pointer are not freed! The plugin itself does the work */
}

std::shared_ptr<IEntity> EntityManager::getObjectByName(const std::string& name) const
{
	for (const auto& e : mObjects) {
		if (e->name() == name)
			return e;
	}

	return nullptr;
}

std::shared_ptr<IEntity> EntityManager::getObjectByName(const std::string& name, const std::string& type) const
{
	for (const auto& e : mObjects) {
		if (e->name() == name && e->type() == type)
			return e;
	}

	return nullptr;
}

void EntityManager::loadFactory(const Registry& reg,
								const std::string& base, const std::string& name)
{
	IPlugin* plugin = PluginLoader::load(base + "pr_pl_" + name, reg);
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load entity plugin " << name << " with base path " << base << std::endl;
		return;
	}

	if (plugin->type() != PT_ENTITY) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a entity plugin!" << std::endl;
		return;
	}

	IEntityFactory* fac = dynamic_cast<IEntityFactory*>(plugin);
	if (!fac) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a entity plugin even when it says it is!" << std::endl;
		return;
	}

	auto names = fac->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Entity with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = fac;
	}
}

} // namespace PR
