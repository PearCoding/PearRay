#pragma once

#include "IMaterialFactory.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB MaterialManager : public AbstractManager<IMaterial, IMaterialFactory> {
public:
	MaterialManager();
	virtual ~MaterialManager();
};
} // namespace PR
