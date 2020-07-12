#pragma once

#include "PR_Config.h"

#if defined(PR_DEBUG)
#ifndef PR_NO_FP_EXCEPTIONS
#define PR_USE_FP_EXCEPTIONS
#endif
#endif

#ifdef PR_USE_FP_EXCEPTIONS
#ifdef PR_OS_LINUX
#include <fenv.h>
#endif
#endif

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
// And enable floating point exceptions if in debug
inline void setupFloatingPointEnvironment()
{
#if defined(PR_USE_HW_FEATURE_SSE2)
#ifndef PR_WITH_FP_DENORMALS
	_mm_setcsr(_mm_getcsr() | (_MM_FLUSH_ZERO_ON | _MM_DENORMALS_ZERO_ON));
#endif
#endif

#ifdef PR_USE_FP_EXCEPTIONS
#ifdef PR_OS_LINUX
	// Embree3 is throwing SIGFPEs on Linux... can not use it for now.
	//feenableexcept(FE_DIVBYZERO | FE_INVALID /*| FE_OVERFLOW*/);
#endif
#endif

}
} // namespace PR
