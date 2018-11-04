#pragma once

#include "AbstractManager.h"

namespace PR {
class IInfiniteLight;
class IInfiniteLightFactory;

class PR_LIB InfiniteLightManager : public AbstractManager<IInfiniteLight, IInfiniteLightFactory> {
public:
	InfiniteLightManager();
	virtual ~InfiniteLightManager();

	bool loadFactory(const RenderManager& mng,
					 const std::string& base, const std::string& name) override;
};
} // namespace PR
