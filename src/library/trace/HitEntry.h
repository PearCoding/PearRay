#pragma once

#include "PR_Config.h"
#include <array>
#include <vector>

namespace PR {
struct PR_LIB_INLINE HitEntry {
public:
	uint32 RayID;
	uint32 MaterialID;
	uint32 EntityID;
	uint32 PrimitiveID;
	float UV[2];
	uint8 Flags;
};
} // namespace PR
