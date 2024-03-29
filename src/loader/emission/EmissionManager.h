#pragma once

#include "IEmissionPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER EmissionManager : public AbstractManager<IEmissionPlugin> {
public:
	EmissionManager();
	virtual ~EmissionManager();
};
} // namespace PR
