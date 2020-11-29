#pragma once

#include "IEntityPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER EntityManager : public AbstractManager<IEntityPlugin> {
public:
	EntityManager();
	virtual ~EntityManager();
};
} // namespace PR
