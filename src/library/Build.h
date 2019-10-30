#pragma once

#include "PR_Config.h"
#include "Version.h"

//OS
#if defined(PR_OS_LINUX)
# define PR_OS_NAME "Linux"
#elif defined(PR_OS_WINDOWS_32)
#  define PR_OS_NAME "Microsoft Windows 32 Bit"
#elif defined(PR_OS_WINDOWS_64)
#  define PR_OS_NAME "Microsoft Windows 64 Bit"
#else
#  define PR_OS_NAME "Unknown"
#endif

//Compiler
#if defined(PR_CC_CYGWIN)
# define PR_CC_NAME "Cygwin"
#endif

#if defined(PR_CC_GNU)
# define PR_CC_NAME "GNU C/C++"
#endif

#if defined(PR_CC_MINGW32)
# if !defined(PR_CC_GNU)
#  define PR_CC_NAME "MinGW 32"
# else
#  undef PR_CC_NAME
#  define PR_CC_NAME "GNU C/C++(MinGW 32)"
# endif
#endif

#if defined(PR_CC_INTEL)
# define PR_CC_NAME "Intel C/C++"
#endif

#if defined(PR_CC_MSC)
# define PR_CC_NAME "Microsoft Visual C++"
#endif

#if !defined(PR_CC_NAME)
# define PR_CC_NAME "Unknown compiler"
#endif

#if defined(PR_DEBUG)
# define PR_BUILDVARIANT_NAME "Debug"
# define PR_BUILD_STRING PR_NAME_STRING " " PR_VERSION_STRING " build variant " PR_BUILDVARIANT_NAME " on " __DATE__  " at " __TIME__ " with " PR_CC_NAME " { OS: " PR_OS_NAME "; Branch: " PR_GIT_BRANCH "; Rev: " PR_GIT_REVISION "}\n"
#else
# define PR_BUILDVARIANT_NAME "Release"
# define PR_BUILD_STRING PR_NAME_STRING " " PR_VERSION_STRING " build variant " PR_BUILDVARIANT_NAME " { OS: " PR_OS_NAME " }\n"
#endif