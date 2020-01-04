#pragma once

#include "parameter/ParameterGroup.h"

namespace PR {
class Environment;
class ParameterGroup;
struct PR_LIB_UTILS SceneLoadContext {
	Environment* Env = nullptr;
	ParameterGroup Parameters;
	std::vector<std::wstring> FileStack;

	inline std::wstring currentFile() const
	{
		return FileStack.empty() ? L"" : FileStack.back();
	}
};
} // namespace PR
