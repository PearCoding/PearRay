#pragma once

#include "AbstractDatabase.h"
#include "IIntegratorPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IIntegratorFactory;

class PR_LIB_LOADER IntegratorManager : public AbstractManager<IIntegratorPlugin> {
public:
	IntegratorManager();
	virtual ~IntegratorManager();
};
} // namespace PR
