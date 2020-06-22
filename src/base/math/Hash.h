#pragma once

#include "PR_Config.h"

#include <functional>

namespace PR {

template <typename T1, typename T2>
inline constexpr size_t hash_union(const T1& v1, const T2& v2)
{
	return std::hash<T1>()(v1) ^ (std::hash<T2>()(v2) << 1);
}

template <typename T>
inline void hash_combine(size_t& seed, const T& val)
{
	seed ^= (std::hash<T>()(val) << 1);
}

template <typename It>
inline size_t hash_range(It first, It last)
{
	size_t seed = 0;
	for (; first != last; ++first)
		hash_combine(seed, *first);
	return seed;
}

} // namespace PR