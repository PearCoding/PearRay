#pragma once

#include "IFilterPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IFilterFactory;

class PR_LIB_UTILS FilterManager : public AbstractManager<IFilterFactory, IFilterPlugin> {
public:
	FilterManager();
	virtual ~FilterManager();
};
} // namespace PR
