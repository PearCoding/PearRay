#pragma once

#include "AbstractManager.h"

namespace PR {
class ILight;
class ILightFactory;

class PR_LIB LightManager : public AbstractManager<ILight, ILightFactory> {
public:
	LightManager();
	virtual ~LightManager();

	bool loadFactory(const RenderManager& mng,
					 const std::string& base, const std::string& name) override;
};
} // namespace PR
