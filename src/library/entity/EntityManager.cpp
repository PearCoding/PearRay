#include "EntityManager.h"
#include "IEntity.h"
#include "IEntityFactory.h"
#include "plugin/PluginManager.h"
#include "renderer/RenderManager.h"

#include "Logger.h"

namespace PR {
EntityManager::EntityManager()
	: AbstractManager()
{
}

EntityManager::~EntityManager()
{
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

bool EntityManager::loadFactory(const RenderManager& mng,
								const std::string& base, const std::string& name)
{
	auto plugin = mng.pluginManager()->load(base + "pr_pl_" + name, *mng.registry());
	if (!plugin) {
		PR_LOG(L_ERROR) << "Couldn't load entity plugin " << name << " with base path " << base << std::endl;
		return false;
	}

	if (plugin->type() != PT_ENTITY) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a entity plugin!" << std::endl;
		return false;
	}

	auto fac = std::dynamic_pointer_cast<IEntityFactory>(plugin);
	if (!fac) {
		PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not a entity plugin even when it says it is!" << std::endl;
		return false;
	}

	auto names = fac->getNames();
	for (const std::string& alias : names) {
		if (mFactories.count(alias))
			PR_LOG(L_WARNING) << "Entity with name " << alias << " already given! Replacing it." << std::endl;

		mFactories[alias] = fac;
	}
	return true;
}

} // namespace PR
