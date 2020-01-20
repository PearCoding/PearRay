#include "Build.h"
#include "Profiler.h"
#include "TriangleOptions.h"
#include "config/Version.h"

#include <sstream>
#include <vector>

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
Version getVersion() { return Version{ PR_VERSION_MAJOR, PR_VERSION_MINOR }; }
std::string getVersionString() { return PR_VERSION_STRING; }
std::string getGitString() { return PR_GIT_BRANCH " " PR_GIT_REVISION; }
std::string getCopyrightString() { return PR_NAME_STRING " " PR_VERSION_STRING " (C) " PR_VENDOR_STRING; }

std::string getCompilerName() { return PR_CC_NAME; }
std::string getOSName() { return PR_OS_NAME; }
std::string getBuildVariant() { return PR_BUILDVARIANT_NAME; }
std::string getFeatureSet()
{
	std::vector<std::string> list;
	// Skip basic features required by x64 anyway
	/*#ifdef PR_HAS_HW_FEATURE_MMX
	list.emplace_back("MMX");
#endif
#ifdef PR_HAS_HW_FEATURE_SSE
	list.emplace_back("SSE");
#endif
#ifdef PR_HAS_HW_FEATURE_SSE2
	list.emplace_back("SSE2");
#endif*/

#ifdef PR_HAS_HW_FEATURE_SSE3
	list.emplace_back("SSE3");
#endif
#ifdef PR_HAS_HW_FEATURE_SSSE3
	list.emplace_back("SSSE3");
#endif
#ifdef PR_HAS_HW_FEATURE_SSE4_1
	list.emplace_back("SSE4.1");
#endif
#ifdef PR_HAS_HW_FEATURE_SSE4_2
	list.emplace_back("SSE4.2");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX
	list.emplace_back("AVX");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX2
	list.emplace_back("AVX2");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_F
	list.emplace_back("AVX512_F");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_DQ
	list.emplace_back("AVX512_DQ");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_IFMA
	list.emplace_back("AVX512_IFMA");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_PF
	list.emplace_back("AVX512_PF");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_ER
	list.emplace_back("AVX512_ER");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_CD
	list.emplace_back("AVX512_CD");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_BW
	list.emplace_back("AVX512_BW");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_VL
	list.emplace_back("AVX512_VL");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_VBMI
	list.emplace_back("AVX512_VBMI");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_VBMI2
	list.emplace_back("AVX512_VBMI2");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_VNNI
	list.emplace_back("AVX512_VNNI");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_BITALG
	list.emplace_back("AVX512_BITALG");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_VPOPCNTDQ
	list.emplace_back("AVX512_VPOPCNTDQ");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_4VNNIW
	list.emplace_back("AVX512_4VNNIW");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_4FMAPS
	list.emplace_back("AVX512_4FMAPS");
#endif
#ifdef PR_HAS_HW_FEATURE_AVX512_BF16
	list.emplace_back("AVX512_BF16");
#endif
#ifdef PR_HAS_HW_FEATURE_HLE
	list.emplace_back("HLE");
#endif
#ifdef PR_HAS_HW_FEATURE_RTM
	list.emplace_back("RTM");
#endif
#ifdef PR_HAS_HW_FEATURE_FMA
	list.emplace_back("FMA3");
#endif
#ifdef PR_HAS_HW_FEATURE_FMA4
	list.emplace_back("FMA4");
#endif
#ifdef PR_HAS_HW_FEATURE_POPCNT
	list.emplace_back("POPCNT");
#endif

	if (list.empty())
		return "NONE";

	std::stringstream stream;
	for (size_t i = 0; i < (list.size() - 1); ++i)
		stream << list.at(i) << "; ";
	stream << list.back();
	return stream.str();
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
		   << "} [Features:<" << getFeatureSet()
		   << ">; Asserts: " << hasAsserts
		   << "; Profile: " << hasProfiler
		   << "; PackN: " << PR_SIMD_BANDWIDTH
		   << "; TriCache: " << hasTriCache
		   << "; TriIntM: " << PR_TRIANGLE_INTERSECTION_METHOD
		   << "]";
	return stream.str();
}
} // namespace Build
} // namespace PR