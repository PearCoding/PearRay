#pragma once

#include "PR_Config.h"

#include <atomic>

namespace PR {
class PR_LIB_CORE RenderThreadStatistics {
public:
	RenderThreadStatistics();

	inline void addTileCount(uint64 i = 1) { mTileCount += i; }
	inline uint64 tileCount() const { return mTileCount; }

private:
	std::atomic<uint64> mTileCount;
};
} // namespace PR
