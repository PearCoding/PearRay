#pragma once

#include "IIntegratorFactory.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IIntegrator;

class PR_LIB_UTILS IntegratorManager : public AbstractManager<IIntegrator, IIntegratorFactory> {
public:
	IntegratorManager();
	virtual ~IntegratorManager();
};
} // namespace PR
