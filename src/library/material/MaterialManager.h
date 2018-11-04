#pragma once

#include "AbstractManager.h"

namespace PR {
class IMaterial;
class IMaterialFactory;

class PR_LIB MaterialManager : public AbstractManager<IMaterial, IMaterialFactory> {
public:
	MaterialManager();
	virtual ~MaterialManager();

	bool loadFactory(const RenderManager& mng,
					 const std::string& base, const std::string& name) override;
};
} // namespace PR
