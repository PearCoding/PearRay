#pragma once

#include "PR_Config.h"

namespace PR {
namespace Projection {
// Map [0, 1] uniformly to [min, max] as integers! (max is included)
template <typename T>
inline T map(float u, T min, T max)
{
	return std::min<T>(max - min, static_cast<T>(u * (max - min + 1))) + min;
}

inline float stratified(float u, int index, int groups, float min = 0, float max = 1)
{
	float range = (max - min) / groups;
	return min + u * range + index * range;
}
} // namespace Projection
} // namespace PR
