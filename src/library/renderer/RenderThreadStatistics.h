#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB RenderThreadStatistics {
public:
	RenderThreadStatistics();

	inline void addTileCount(uint64 i = 1) { mTileCount += i; }
	inline uint64 tileCount() const { return mTileCount; }

private:
	uint64 mTileCount;
};
} // namespace PR
