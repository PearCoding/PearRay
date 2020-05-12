#pragma once

#include "PR_Config.h"

namespace PR {
/* Visual feedback tile mode */
enum TileMode {
	TM_LINEAR = 0,
	TM_TILE,
	TM_SPIRAL
};

enum TimeMappingMode {
	TMM_CENTER = 0, // [0.5, 0.5]
	TMM_LEFT,		// [-1, 0]
	TMM_RIGHT		// [0, 1]
};
} // namespace PR
