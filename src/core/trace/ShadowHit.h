#pragma once

#include "PR_Config.h"

namespace PR {
struct PR_LIB_CORE ShadowHit {
public:
	bool Successful;
	Vector3f Parameter;
	uint32 EntityID;
	uint32 PrimitiveID;
};
} // namespace PR
