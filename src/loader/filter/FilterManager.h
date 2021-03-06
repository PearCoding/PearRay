#pragma once

#include "AbstractDatabase.h"
#include "IFilterPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class IFilterFactory;
class Environment;

class PR_LIB_LOADER FilterManager : public AbstractManager<IFilterPlugin> {
public:
	FilterManager();
	virtual ~FilterManager();

	bool createDefaultsIfNecessary(Environment* env);
};
} // namespace PR
