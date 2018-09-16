#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB LightPathManager {
public:
	LightPathManager()  = default;
	~LightPathManager() = default;

	static size_t getLabelIndex(const std::string& lbl);
};
} // namespace PR