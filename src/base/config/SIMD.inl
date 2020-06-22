// IWYU pragma: private, include "PR_Config.h"
#include "config/HW_Switch.inl"
#include "config/HW_Features.inl"

// SIMD
// Always enable SSE2... everything below is too old anyway
#define PR_USE_HW_FEATURE_SSE2

#if defined(PR_HAS_HW_FEATURE_SSE3) && !defined(PR_DISABLE_HW_FEATURE_SSE3)
#define PR_USE_HW_FEATURE_SSE3
#endif

#if defined(PR_HAS_HW_FEATURE_SSSE3) && !defined(PR_DISABLE_HW_FEATURE_SSSE3)
#define PR_USE_HW_FEATURE_SSSE3
#endif

#if defined(PR_HAS_HW_FEATURE_SSE4_1) && !defined(PR_DISABLE_HW_FEATURE_SSE4_1)
#define PR_USE_HW_FEATURE_SSE4_1
#endif

#if defined(PR_HAS_HW_FEATURE_SSE4_2) && !defined(PR_DISABLE_HW_FEATURE_SSE4_2)
#define PR_USE_HW_FEATURE_SSE4_2
#endif

#if defined(PR_HAS_HW_FEATURE_AVX) && !defined(PR_DISABLE_HW_FEATURE_AVX)
#define PR_USE_HW_FEATURE_AVX
#endif

#if defined(PR_HAS_HW_FEATURE_AVX2) && !defined(PR_DISABLE_HW_FEATURE_AVX2)
#define PR_USE_HW_FEATURE_AVX2
#endif

#if defined(PR_HAS_HW_FEATURE_AVX512F) && !defined(PR_DISABLE_HW_FEATURE_AVX512F)
#define PR_USE_HW_FEATURE_AVX512F
#endif

#if defined(PR_HAS_HW_FEATURE_AVX512BW) && !defined(PR_DISABLE_HW_FEATURE_AVX512BW)
#define PR_USE_HW_FEATURE_AVX512BW
#endif

#if defined(PR_HAS_HW_FEATURE_AVX512DQ) && !defined(PR_DISABLE_HW_FEATURE_AVX512DQ)
#define PR_USE_HW_FEATURE_AVX512DQ
#endif

#if defined(PR_HAS_HW_FEATURE_AVX512VL) && !defined(PR_DISABLE_HW_FEATURE_AVX512VL)
#define PR_USE_HW_FEATURE_AVX512VL
#endif

#if defined(PR_HAS_HW_FEATURE_POPCNT) && !defined(PR_DISABLE_HW_FEATURE_POPCNT)
#define PR_USE_HW_FEATURE_POPCNT
#endif

#if defined(PR_HAS_HW_FEATURE_FMA4) && !defined(PR_DISABLE_HW_FEATURE_FMA4)
#define PR_USE_HW_FEATURE_FMA4
#elif defined(PR_HAS_HW_FEATURE_FMA) && !defined(PR_DISABLE_HW_FEATURE_FMA)
#define PR_USE_HW_FEATURE_FMA3
#endif

#ifdef PR_USE_HW_FEATURE_AVX512F
#define PR_SIMD_ALIGNMENT_PARAM (64)
#elif defined PR_USE_HW_FEATURE_AVX
#define PR_SIMD_ALIGNMENT_PARAM (32)
#else
#define PR_SIMD_ALIGNMENT_PARAM (16)
#endif

#define PR_SIMD_ALIGN alignas(PR_SIMD_ALIGNMENT_PARAM)