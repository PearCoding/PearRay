#pragma once

#include "AbstractDatabase.h"
#include "ISpectralMapperPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class ISpectralMapperFactory;
class Environment;

class PR_LIB_LOADER SpectralMapperManager : public AbstractManager<ISpectralMapperPlugin> {
public:
	SpectralMapperManager();
	virtual ~SpectralMapperManager();

	bool createDefaultsIfNecessary(Environment* env);
};
} // namespace PR
