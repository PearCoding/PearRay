#pragma once

#include "PR_Config.h"

namespace PR {
// Default position getter
template <typename T>
struct position_getter;

// Simple position getter
template <>
struct position_getter<Vector3f> {
	inline Vector3f operator()(const Vector3f& p) const
	{
		return p;
	}
};

} // namespace PR
