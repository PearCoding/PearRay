#pragma once

#include "PR_Config.h"

#include <memory>
#include <vector>

namespace PR {
class PR_LIB_BASE MemoryPool final {
public:
	inline MemoryPool(size_t elementSize, size_t alignment, size_t maxElements);
	inline ~MemoryPool();

	inline void* allocate() noexcept;
	inline void deallocate(void* ptr) noexcept;

	inline void freeAll() noexcept;

	template <typename T, class... Args>
	inline T* create(Args... args);
	template <typename T>
	inline void destroy(T* ptr) noexcept(std::is_nothrow_destructible<T>::value);

	inline size_t usedMemory() const noexcept { return mUsedMemory; }
	inline size_t maxMemory() const noexcept { return mMemory.size(); }
	inline size_t elementSize() const noexcept { return mElementSize; }
	inline size_t elementAlignment() const noexcept { return mElementAlignment; }
	inline size_t maxElements() const noexcept { return maxMemory() / blockSize(); }
	inline size_t blockSize() const noexcept { return mBlockSize; }
	inline uint32 elementCount() const noexcept { return mElementCount; }

	inline bool isEmpty() const noexcept { return usedMemory() == 0; }
	inline bool isFull() const noexcept { return usedMemory() == maxMemory(); }

private:
	size_t mElementSize		 = 0;
	size_t mElementAlignment = 0;
	size_t mBlockSize		 = 0;
	uint32 mElementCount	 = 0;
	size_t mUsedMemory		 = 0;
	size_t mFirstFreeElement = 0;
	std::vector<uint8> mMemory;
};
} // namespace PR

#include "MemoryPool.inl"