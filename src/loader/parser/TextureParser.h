#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class SceneLoadContext;
class SceneLoader;
class PR_LIB_LOADER TextureParser {
public:
	void parse(const SceneLoadContext& ctx, const std::string& name, const DL::DataGroup& group) const;
};
} // namespace PR
