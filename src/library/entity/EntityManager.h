#pragma once

#include "AbstractManager.h"

namespace PR {
class IEntity;
class IEntityFactory;

class PR_LIB EntityManager : public AbstractManager<IEntity, IEntityFactory> {
public:
	EntityManager();
	virtual ~EntityManager();

	std::shared_ptr<IEntity> getObjectByName(const std::string& name) const;
	std::shared_ptr<IEntity> getObjectByName(const std::string& name, const std::string& type) const;
	bool loadFactory(const RenderManager& mng,
					 const std::string& base, const std::string& name) override;
};
} // namespace PR
