#pragma once

#include "PR_Config.h"

#include <memory>
#include <vector>

namespace PR {
class PR_LIB_BASE MemoryStack final {
public:
	inline explicit MemoryStack(size_t maxSize);
	inline ~MemoryStack();

	inline void* allocate(size_t size, size_t alignment = 1) noexcept;
	inline void mark();

	inline void freeUntilMarker() noexcept;
	inline void freeAll() noexcept;

	template <typename T, class... Args>
	inline T* create(Args... args);

	inline size_t usedMemory() const noexcept { return mUsedMemory; }
	inline size_t maxMemory() const noexcept { return mMemory.size(); }
	inline uint32 markerCount() const noexcept { return mMarkerCount; }

	inline bool isEmpty() const noexcept { return usedMemory() == 0; }
	inline bool isFull() const noexcept { return usedMemory() == maxMemory(); }

private:
	using marker_t = size_t;
	inline marker_t* allocateMarker();

	marker_t mLastMarker = 0;
	uint32 mMarkerCount	 = 0;
	size_t mUsedMemory	 = 0;
	std::vector<uint8> mMemory;
};
} // namespace PR

#include "MemoryStack.inl"