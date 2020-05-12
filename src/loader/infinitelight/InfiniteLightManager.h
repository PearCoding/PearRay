#pragma once

#include "IInfiniteLightPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER InfiniteLightManager : public AbstractManager<IInfiniteLight, IInfiniteLightPlugin> {
public:
	InfiniteLightManager();
	virtual ~InfiniteLightManager();
};
} // namespace PR
