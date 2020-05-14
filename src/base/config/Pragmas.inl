// IWYU pragma: private, include "PR_Config.h"

// clang-format off
#if defined(PR_CC_CLANG)
#define PR_OPT_LOOP                         \
	_Pragma("clang loop vectorize(enable)") \
	_Pragma("clang loop interleave(enable)")
#elif defined(PR_CC_GNU)
#define PR_OPT_LOOP \
    _Pragma("GCC ivdep")
#elif defined(PR_CC_MSC)
#define PR_OPT_LOOP \
    _Pragma("loop(ivdep)")
#else
#define PR_OPT_LOOP 
#endif
// clang-format on