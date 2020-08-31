// IWYU pragma: private, include "PR_Config.h"

// TODO: Better, more adaptive solution available?

#ifndef PR_LIB_BASE_STATIC
#if defined(PR_LIB_BASE_BUILD)
#define PR_LIB_BASE PR_EXPORT
#else
#define PR_LIB_BASE PR_IMPORT
#endif
#else
#define PR_LIB_BASE
#endif

#ifndef PR_LIB_CORE_STATIC
#if defined(PR_LIB_CORE_BUILD)
#define PR_LIB_CORE PR_EXPORT
#else
#define PR_LIB_CORE PR_IMPORT
#endif
#else
#define PR_LIB_CORE
#endif

#ifndef PR_LIB_LOADER_STATIC
#if defined(PR_LIB_LOADER_BUILD)
#define PR_LIB_LOADER PR_EXPORT
#else
#define PR_LIB_LOADER PR_IMPORT
#endif
#else
#define PR_LIB_LOADER
#endif

#ifndef PR_LIB_UI_STATIC
#if defined(PR_LIB_UI_BUILD)
#define PR_LIB_UI PR_EXPORT
#else
#define PR_LIB_UI PR_IMPORT
#endif
#else
#define PR_LIB_UI
#endif