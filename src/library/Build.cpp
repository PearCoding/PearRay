#include "Build.h"
#include "Profiler.h"
#include "TriangleOptions.h"
#include "Version.h"

//OS
#if defined(PR_OS_LINUX)
#define PR_OS_NAME "Linux"
#elif defined(PR_OS_WINDOWS_32)
#define PR_OS_NAME "Microsoft Windows 32 Bit"
#elif defined(PR_OS_WINDOWS_64)
#define PR_OS_NAME "Microsoft Windows 64 Bit"
#else
#define PR_OS_NAME "Unknown"
#endif

//Compiler
#if defined(PR_CC_CYGWIN)
#define PR_CC_NAME "Cygwin"
#endif

#if defined(PR_CC_GNU)
#define PR_CC_NAME "GNU C/C++"
#endif

#if defined(PR_CC_MINGW32)
#if !defined(PR_CC_GNU)
#define PR_CC_NAME "MinGW 32"
#else
#undef PR_CC_NAME
#define PR_CC_NAME "GNU C/C++(MinGW 32)"
#endif
#endif

#if defined(PR_CC_INTEL)
#define PR_CC_NAME "Intel C/C++"
#endif

#if defined(PR_CC_MSC)
#define PR_CC_NAME "Microsoft Visual C++"
#endif

#if !defined(PR_CC_NAME)
#define PR_CC_NAME "Unknown"
#endif

#if defined(PR_DEBUG)
#define PR_BUILDVARIANT_NAME "Debug"
#else
#define PR_BUILDVARIANT_NAME "Release"
#endif

namespace PR {
namespace Build {
std::string getCompilerName() { return PR_CC_NAME; }
std::string getOSName() { return PR_OS_NAME; }
std::string getBuildVariant() { return PR_BUILDVARIANT_NAME; }
std::string getSIMDLevel()
{
#if defined SIMDPP_USE_AVX2
	return "AVX2";
#elif defined SIMDPP_USE_AVX
	return "AVX";
#elif defined SIMDPP_USE_SSE4_1
	return "SSE4.1"
#elif defined SIMDPP_USE_SSSE3
	return "SSSE3";
#elif defined SIMDPP_USE_SSE3
	return "SSE3";
#elif defined SIMDPP_USE_SSE2
	return "SSE2";
#else
	return "None";
#endif
}

std::string getBuildString()
{
#ifdef PR_NO_ASSERTS
	constexpr bool hasAsserts = false;
#else
	constexpr bool hasAsserts  = true;
#endif
#ifdef PR_WITH_PROFILER
	constexpr bool hasProfiler = true;
#else
	constexpr bool hasProfiler = false;
#endif
#ifdef PR_TRIANGLE_USE_CACHE
	constexpr bool hasTriCache = true;
#else
	constexpr bool hasTriCache = false;
#endif

	std::stringstream stream;
	stream << std::boolalpha
		   << PR_NAME_STRING << " " << PR_VERSION_STRING
		   << " (" << getBuildVariant()
		   << ") on " __DATE__ " at " __TIME__
		   << " with " << getCompilerName()
		   << " { OS: " << getOSName()
		   << "; Branch: " PR_GIT_BRANCH
		   << "; Rev: " PR_GIT_REVISION
		   << "} [SIMD: " << getSIMDLevel()
		   << "; Asserts: " << hasAsserts
		   << "; Profile: " << hasProfiler
		   << "; PackN: " << PR_SIMD_BANDWIDTH
		   << "; TriCache: " << hasTriCache
		   << "; TriIntM: " << PR_TRIANGLE_INTERSECTION_METHOD
		   << "]";
	return stream.str();
}
} // namespace Build
} // namespace PR