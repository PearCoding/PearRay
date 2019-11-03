#pragma once

#include "IEmissionFactory.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_UTILS EmissionManager : public AbstractManager<IEmission, IEmissionFactory> {
public:
	EmissionManager();
	virtual ~EmissionManager();
};
} // namespace PR
