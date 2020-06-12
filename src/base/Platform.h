#pragma once

#include "PR_Config.h"

namespace PR {
// Ensure that fstreams are opened with this! fstream(encodePath(path))!
#ifdef PR_OS_WINDOWS
PR_LIB_BASE std::wstring encodePath(const std::wstring& path);
PR_LIB_BASE std::wstring encodePath(const std::string& path);
#else
PR_LIB_BASE std::string encodePath(const std::wstring& path);
PR_LIB_BASE std::string encodePath(const std::string& path);
#endif

// Force flush to zero for denormal/subnormal values
inline void setupFloatingPointFlushBehaviour()
{
#if defined(PR_USE_HW_FEATURE_SSE2)
	_mm_setcsr(_mm_getcsr() | (_MM_FLUSH_ZERO_ON | _MM_DENORMALS_ZERO_ON));
#endif
}
} // namespace PR
