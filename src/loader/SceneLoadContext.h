#pragma once

#include "parameter/ParameterGroup.h"

#include <filesystem>

namespace PR {
class Environment;
class ParameterGroup;
struct PR_LIB_LOADER SceneLoadContext {
	Environment* Env = nullptr;
	ParameterGroup Parameters;
	std::vector<std::filesystem::path> FileStack;

	inline std::filesystem::path currentFile() const
	{
		return FileStack.empty() ? L"" : FileStack.back();
	}

	inline std::filesystem::path escapePath(const std::filesystem::path& path) const {
		if(path.is_absolute())
			return path;
		
		if(std::filesystem::exists(path))
			return path;
		
		return std::filesystem::absolute(currentFile().parent_path() / path);
	}
};
} // namespace PR
