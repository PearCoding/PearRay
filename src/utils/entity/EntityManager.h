#pragma once

#include "IEntityFactory.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_UTILS EntityManager : public AbstractManager<IEntity, IEntityFactory> {
public:
	EntityManager();
	virtual ~EntityManager();

	std::shared_ptr<IEntity> getObjectByName(const std::string& name) const;
	std::shared_ptr<IEntity> getObjectByName(const std::string& name, const std::string& type) const;
};
} // namespace PR
