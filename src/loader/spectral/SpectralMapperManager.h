#pragma once

#include "ISpectralMapperPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class ISpectralMapperFactory;
class Environment;

class PR_LIB_LOADER SpectralMapperManager : public AbstractManager<ISpectralMapperFactory, ISpectralMapperPlugin> {
public:
	SpectralMapperManager();
	virtual ~SpectralMapperManager();

	bool createDefaultsIfNecessary(Environment* env);
};
} // namespace PR
