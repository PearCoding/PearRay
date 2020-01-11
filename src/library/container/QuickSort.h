#pragma once

#include "math/Bits.h"

namespace PR {

/* Generic inplace quick sort algorithm
 */
template <typename VT, typename S>
inline PR_LIB  void quickSort(VT* visitor, S swapper, size_t first, size_t last)
{
	if (first >= last)
		return;

	size_t i = first;
	size_t j = last - 1;
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
