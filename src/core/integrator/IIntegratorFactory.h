#pragma once

#include "PR_Config.h"

namespace PR {
class IIntegrator;
class PR_LIB_CORE IIntegratorFactory {
public:
	virtual ~IIntegratorFactory() = default;

	virtual std::shared_ptr<IIntegrator> createInstance() const = 0;
};
} // namespace PR
