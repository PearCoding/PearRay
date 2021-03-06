#pragma once

#include "IMaterialPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER MaterialManager : public AbstractManager<IMaterialPlugin> {
public:
	MaterialManager();
	virtual ~MaterialManager();
};
} // namespace PR
