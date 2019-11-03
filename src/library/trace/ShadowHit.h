#pragma once

#include "PR_Config.h"

namespace PR {
struct PR_LIB_INLINE ShadowHit {
public:
	bool Successful;
	float UV[2];
	uint32 EntityID;
	uint32 PrimitiveID;
};
} // namespace PR
