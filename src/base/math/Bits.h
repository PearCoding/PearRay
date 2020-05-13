#pragma once

#include "PR_Config.h"

namespace PR {

/* Undefined if v == 0 */
inline uint32 clz(uint32 v)
{
#ifdef PR_CC_GNU
	return __builtin_clz(v);
#elif defined(PR_CC_MSC)
	retirm __lzcnt(v);
#else
	uint32 r = 0;
	while (v >>= 1)
		r++;
	return r;
#endif
}

// Most Significant Bit
// Sets the most significant bit bounded by v
inline uint32 msb(uint32 v)
{
	return 1 << (32 - clz(v));
}

// Value Effect Mask
// Set all bits till the left most bit of v
inline uint32 vem(uint32 v)
{
	uint32 max = msb(v);
	return max | (max - 1);
}
} // namespace PR