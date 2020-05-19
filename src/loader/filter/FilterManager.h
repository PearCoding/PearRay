#pragma once

#include "IFilterPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IFilterFactory;
class Environment;

class PR_LIB_LOADER FilterManager : public AbstractManager<IFilterFactory, IFilterPlugin> {
public:
	FilterManager();
	virtual ~FilterManager();

	bool createDefaultsIfNecessary(Environment* env);
};
} // namespace PR
