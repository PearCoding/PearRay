#pragma once

#include "ISamplerPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class ISamplerFactory;

class PR_LIB_UTILS SamplerManager : public AbstractManager<ISamplerFactory, ISamplerPlugin> {
public:
	SamplerManager();
	virtual ~SamplerManager();
};
} // namespace PR
