#pragma once

#include "PR_Config.h"

namespace PR {

/* Undefined if v == 0 */
template <typename T>
inline unsigned int clz(T v)
{
#ifdef PR_CC_GNU
	return __builtin_clz(v);
#else
	unsigned int r = 0;
	while (v >>= 1) {
		r++;
	}
	return r;
#endif
}
} // namespace PR