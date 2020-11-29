#pragma once

#include "IInfiniteLightPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER InfiniteLightManager : public AbstractManager<IInfiniteLightPlugin> {
public:
	InfiniteLightManager();
	virtual ~InfiniteLightManager();
};
} // namespace PR
