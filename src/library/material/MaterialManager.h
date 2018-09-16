#pragma once

#include "AbstractManager.h"

namespace PR {
class IMaterial;
class IMaterialFactory;

class PR_LIB MaterialManager : public AbstractManager<IMaterial, IMaterialFactory> {
public:
	MaterialManager();
	virtual ~MaterialManager();

	void loadFactory(const Registry& reg,
					 const std::string& base, const std::string& name) override;
};
} // namespace PR
