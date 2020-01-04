#pragma once

#include "IEmissionPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_UTILS EmissionManager : public AbstractManager<IEmission, IEmissionPlugin> {
public:
	EmissionManager();
	virtual ~EmissionManager();
};
} // namespace PR
