#pragma once

#include "AbstractDatabase.h"
#include "ISamplerPlugin.h"
#include "plugin/AbstractManager.h"
#include "sampler/ISampler.h"

namespace PR {
class ISamplerFactory;
class Environment;

class PR_LIB_LOADER SamplerManager : public AbstractManager<ISamplerPlugin> {
public:
	SamplerManager();
	virtual ~SamplerManager();

	bool createDefaultsIfNecessary(Environment* env);
};
} // namespace PR
