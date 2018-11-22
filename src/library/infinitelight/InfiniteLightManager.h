#pragma once

#include "IInfiniteLightFactory.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB InfiniteLightManager : public AbstractManager<IInfiniteLight, IInfiniteLightFactory> {
public:
	InfiniteLightManager();
	virtual ~InfiniteLightManager();
};
} // namespace PR
