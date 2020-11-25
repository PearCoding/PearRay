#pragma once

#include "PR_Config.h"

namespace PR {

struct PR_LIB_CORE HitEntry {
public:
	uint32 RayID	   = PR_INVALID_ID;
	//uint32 MaterialID  = 0;
	uint32 EntityID	   = PR_INVALID_ID;
	uint32 PrimitiveID = PR_INVALID_ID;
	Vector3f Parameter = { 0, 0, 0 };
	//uint8 Flags		   = 0;
};
} // namespace PR
