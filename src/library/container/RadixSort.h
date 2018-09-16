#pragma once

#include "math/Bits.h"

namespace PR {

/* Generic inplace radix sort algorithm based on:
 * http://www.drdobbs.com/architecture-and-design/algorithm-improvement-through-performanc/220300654
 */
template <typename VT, typename S>
inline void PR_LIB_INLINE radixSort(VT* visitor, S swapper, uint32 first, uint32 last, VT mask)
{
	uint32 end0 = first;
	uint32 end1 = last;

	while (end0 <= end1) {
		if (0 == (mask & visitor[end0])) {
			++end0;
		} else {
			swapper(end0, end1);
			--end1;
		}
	}

	mask >>= 1;
	if (mask != 0) {
		if (first < (end0 - 1))
			radixSort(visitor, swapper, first, end0 - 1, mask);
		if ((end1 + 1) < last)
			radixSort(visitor, swapper, end1 + 1, last, mask);
	}
}

template <typename VT, typename S>
inline void PR_LIB_INLINE radixSort(VT* visitor, S swapper, uint32 first, uint32 last)
{
	uint32 s = last - first + 1;
	VT mask  = 1 << (32 - clz(s));
	return radixSort(visitor, swapper, first, last, mask);
}
} // namespace PR
