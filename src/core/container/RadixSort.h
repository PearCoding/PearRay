#pragma once

#include "math/Bits.h"

namespace PR {

/* Generic inplace radix sort algorithm based on:
 * http://www.drdobbs.com/architecture-and-design/algorithm-improvement-through-performanc/220300654
 * and
 * https://rosettacode.org/wiki/Sorting_algorithms/Radix_sort#C.2B.2B
 */
template <typename VT, typename S>
inline PR_LIB_CORE void _radixSort(VT* visitor, S swapper, size_t begin, size_t end, VT mask)
{
	if (begin >= end || mask == 0)
		return;

	size_t left	 = begin;
	size_t right = end - 1;

	while (true) {
		while (left < right && !(visitor[left] & mask))
			++left;
		while (left < right && (visitor[right] & mask))
			--right;
		if (left >= right)
			break;
		swapper(left, right);
	}

	if (!(mask & visitor[left]) && left < end)
		++left;

	mask >>= 1;
	_radixSort(visitor, swapper, begin, left, mask);
	_radixSort(visitor, swapper, left, end, mask);
}

template <typename VT, typename S>
inline PR_LIB_CORE void radixSort(VT* visitor, S swapper, size_t first, size_t last, VT mask)
{
	_radixSort(visitor, swapper, first, last + 1, mask);
}

template <typename VT, typename S>
inline void radixSort(VT* visitor, S swapper, size_t first, size_t last)
{
	if (first >= last)
		return;

	size_t s = last - first - 1;
	radixSort(visitor, swapper, first, last, msb((uint32)s));
}
} // namespace PR
