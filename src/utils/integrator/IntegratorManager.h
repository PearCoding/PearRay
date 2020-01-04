#pragma once

#include "IIntegratorPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IIntegratorFactory;

class PR_LIB_UTILS IntegratorManager : public AbstractManager<IIntegratorFactory, IIntegratorPlugin> {
public:
	IntegratorManager();
	virtual ~IntegratorManager();
};
} // namespace PR
