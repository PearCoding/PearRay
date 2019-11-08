#pragma once

#include "PR_Config.h"

namespace PR {
#ifdef PR_OS_WINDOWS
std::wstring encodePath(const std::string& path);
#else
std::string encodePath(const std::string& path);
#endif
} // namespace PR
