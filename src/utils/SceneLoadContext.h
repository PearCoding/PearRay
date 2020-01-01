#pragma once

#include "PR_Config.h"

#include <vector>

namespace PR {
class Environment;
struct PR_LIB_UTILS SceneLoadContext {
	Environment* Env = nullptr;
	std::vector<std::wstring> FileStack;

	inline std::wstring currentFile() const
	{
		return FileStack.empty() ? L"" : FileStack.back();
	}
};
} // namespace PR
