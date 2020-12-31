#pragma once

#include "PR_Config.h"

namespace PR {
/* Visual feedback tile mode */
enum class TileMode {
	Linear = 0,
	Tile,
	Spiral,
	ZOrder
};

enum class TimeMappingMode {
	Center = 0, // [0.5, 0.5]
	Left,		// [-1, 0]
	Right		// [0, 1]
};
} // namespace PR
