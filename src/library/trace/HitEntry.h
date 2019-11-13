#pragma once

#include "PR_Config.h"

namespace PR {
struct PR_LIB HitEntry {
public:
	uint32 RayID		= 0;
	uint32 SessionRayID = 0;
	uint32 MaterialID   = 0;
	uint32 EntityID		= 0;
	uint32 PrimitiveID  = 0;
	float UV[2]			= { 0, 0 };
	uint8 Flags			= 0;
};
} // namespace PR
