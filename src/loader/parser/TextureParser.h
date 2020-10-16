#pragma once

#include "PR_Config.h"

#include <filesystem>

namespace DL {
class DataGroup;
}

namespace PR {
class SceneLoadContext;
class PR_LIB_LOADER TextureParser {
public:
	static void parse(SceneLoadContext& ctx, const std::string& name, const DL::DataGroup& group);
	static void convertToParametric(const SceneLoadContext& ctx, const std::filesystem::path& input, const std::filesystem::path& output);
};
} // namespace PR
