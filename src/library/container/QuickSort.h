#pragma once

#include "math/Bits.h"

namespace PR {

/* Generic inplace quick sort algorithm
 */
template <typename VT, typename S>
inline void PR_LIB quickSort(VT* visitor, S swapper, uint32 first, uint32 last)
{
	if (first >= last)
		return;

	uint32 i = first;
	uint32 j = last - 1;
	VT pivot = visitor[last];

	do {
		while (i < last && visitor[i] < pivot)
			++i;

		while (j > first && visitor[j] >= pivot)
			--j;

		if (i < j)
			swapper(i, j);
	} while (i < j);

	swapper(i, last);

	if (i > 1)
		quickSort(visitor, swapper, first, i - 1);

	quickSort(visitor, swapper, i + 1, last);
}
} // namespace PR
