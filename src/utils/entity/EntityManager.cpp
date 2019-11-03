#include "EntityManager.h"
#include "entity/IEntity.h"

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

} // namespace PR
