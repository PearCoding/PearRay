// IWYU pragma: private, include "PR_Config.h"

#define PR_PRAGMA(x) _Pragma(#x)

// clang-format off
#if defined(PR_CC_CLANG)
#define PR_OPT_LOOP                         \
	PR_PRAGMA(clang loop vectorize(enable)) \
	PR_PRAGMA(clang loop interleave(enable))
#define PR_UNROLL_LOOP(n) PR_PRAGMA(unroll n)
#elif defined(PR_CC_GNU)
#define PR_OPT_LOOP PR_PRAGMA(GCC ivdep)
#define PR_UNROLL_LOOP(n) PR_PRAGMA(GCC unroll n)
#elif defined(PR_CC_MSC)
#define PR_OPT_LOOP PR_PRAGMA(loop(ivdep))
#define PR_UNROLL_LOOP(n) 
#else
#define PR_OPT_LOOP 
#define PR_UNROLL_LOOP(n) 
#endif
// clang-format on