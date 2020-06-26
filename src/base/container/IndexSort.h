#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {
/* Sorts array indices by cyclic permutation
 * and calls swapF for rearrangement of other
 * connected containers.
 */
template <typename SwapF, typename T>
inline PR_LIB_BASE void sortByIndex(SwapF swapF, std::vector<T>& index)
{
	for (size_t i = 0; i < index.size(); ++i) {
		while (index[i] != index[index[i]]) {
			swapF(index[i], index[index[i]]);
			std::swap(index[i], index[index[i]]);
		}
	}
}
} // namespace PR
