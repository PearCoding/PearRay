#pragma once

#include "PR_Config.h"

namespace PR {
// Ensure that fstreams are opened with this! fstream(encodePath(path))!
#ifdef PR_OS_WINDOWS
PR_LIB std::wstring encodePath(const std::wstring& path);
PR_LIB std::wstring encodePath(const std::string& path);
#else
PR_LIB std::string encodePath(const std::wstring& path);
PR_LIB std::string encodePath(const std::string& path);
#endif
} // namespace PR
