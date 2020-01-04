#pragma once

#include "IMaterialPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_UTILS MaterialManager : public AbstractManager<IMaterial, IMaterialPlugin> {
public:
	MaterialManager();
	virtual ~MaterialManager();
};
} // namespace PR
