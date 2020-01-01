#pragma once

#include "PR_Config.h"
#include <string>

namespace PR {

struct SceneLoadContext;
class PR_LIB_UTILS SubGraphLoader {
public:
	virtual void load(const std::wstring& file, const SceneLoadContext& ctx) = 0;
};
} // namespace PR
