#pragma once

#include "IInfiniteLightPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_UTILS InfiniteLightManager : public AbstractManager<IInfiniteLight, IInfiniteLightPlugin> {
public:
	InfiniteLightManager();
	virtual ~InfiniteLightManager();
};
} // namespace PR
