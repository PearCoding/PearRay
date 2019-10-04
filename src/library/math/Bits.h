#pragma once

#include "PR_Config.h"

namespace PR {

/* Undefined if v == 0 */
inline uint32 clz(uint32 v)
{
#ifdef PR_CC_GNU
	return __builtin_clz(v);
#else
	uint32 r = 0;
	while (v >>= 1) {
		r++;
	}
	return r;
#endif
}

// Value Effect Mask
// Set all bits till the left most bit of v
inline uint32 vem(uint32 v) {
	uint32 max   = 1 << (32 - clz(v));
	return max | (max - 1);
}
} // namespace PR