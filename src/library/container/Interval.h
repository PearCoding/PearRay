#pragma once

#include "PR_Config.h"

namespace PR {
namespace Interval {
// Interval has to be sorted
template <typename Predicate>
size_t find(size_t size, const Predicate& pred)
{
	int first = 0;
	int len   = (int)size;

	while (len > 0) {
		int half   = len / 2;
		int middle = first + half;
		if (pred(middle)) {
			first = middle + 1;
			len -= half + 1;
		} else {
			len = half;
		}
	}

	return (size_t)std::max(0, std::min(first - 1, (int)size - 2));
}
} // namespace Interval
} // namespace PR
