#pragma once

#include "PR_Config.h"

namespace PR {
class HitStream;
struct PR_LIB_INLINE ShadingGroup {
	HitStream* Stream;
	uint32 EntityID;
	uint32 MaterialID;
	size_t Start;
	size_t End;
};

} // namespace PR
