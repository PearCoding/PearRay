#pragma once

#include "IEntityPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER EntityManager : public AbstractManager<IEntity, IEntityPlugin> {
public:
	EntityManager();
	virtual ~EntityManager();

	std::shared_ptr<IEntity> getObjectByName(const std::string& name) const;
	std::shared_ptr<IEntity> getObjectByName(const std::string& name, const std::string& type) const;
};
} // namespace PR
