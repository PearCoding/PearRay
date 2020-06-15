#pragma once

#include "PR_Config.h"

namespace PR {
class HitStream;
struct PR_LIB_CORE ShadingGroupBlock {
	HitStream* Stream;
	uint32 EntityID;
	size_t Start;
	size_t End;

	inline size_t size() const { return End - Start + 1; }
};

} // namespace PR
