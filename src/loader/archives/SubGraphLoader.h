#pragma once

#include "PR_Config.h"
#include <filesystem>
#include <string>

namespace PR {

class SceneLoadContext;
class PR_LIB_LOADER SubGraphLoader {
public:
	virtual void load(const std::filesystem::path& file, const SceneLoadContext& ctx) = 0;
};
} // namespace PR
