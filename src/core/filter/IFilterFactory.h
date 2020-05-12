#pragma once

#include "PR_Config.h"

namespace PR {
class IFilter;
class PR_LIB_CORE IFilterFactory {
public:
	virtual ~IFilterFactory() = default;

	virtual std::shared_ptr<IFilter> createInstance() const = 0;
};
} // namespace PR