// IWYU pragma: private, include "memory/MemoryPool.h"
namespace PR {
inline MemoryPool::MemoryPool(size_t elementSize, size_t alignment, size_t maxElements)
	: mElementSize(elementSize)
	, mElementAlignment(alignment)
	, mBlockSize(std::max(sizeof(size_t), elementSize + alignment))
	, mMemory(maxElements * mBlockSize)
{
	freeAll();
}

inline MemoryPool::~MemoryPool() {}

inline void* MemoryPool::allocate() noexcept
{
	if (isFull())
		return nullptr;

	size_t space = mBlockSize;
	uint8* sptr	 = &mMemory[mFirstFreeElement * mBlockSize];
	void* ptr	 = sptr;
	if (std::align(mElementAlignment, mElementSize, ptr, space)) {
		mFirstFreeElement = *reinterpret_cast<size_t*>(sptr);
		mUsedMemory += mBlockSize;
		return ptr;
	} else {
		return nullptr;
	}
}

inline void MemoryPool::deallocate(void* ptr) noexcept
{
	if (!ptr || mUsedMemory < mBlockSize)
		return;

	uintptr_t off	  = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(mMemory.data());
	const size_t elem = off / mBlockSize; // Drop alignment offset

	uint8* uaptr					  = &mMemory[elem * mBlockSize]; // Unaligned pointer
	*reinterpret_cast<size_t*>(uaptr) = mFirstFreeElement;
	mFirstFreeElement				  = elem;

	mUsedMemory -= mBlockSize;
}

inline void MemoryPool::freeAll() noexcept
{
	mUsedMemory		  = 0;
	mElementCount	  = 0;
	mFirstFreeElement = 0;
	size_t elements	  = maxElements();
	for (size_t i = 0; i < elements; ++i)
		*reinterpret_cast<size_t*>(&mMemory[i * mBlockSize]) = i + 1;
}

template <typename T, class... Args>
inline T* MemoryPool::create(Args... args)
{
	PR_ASSERT(sizeof(T) == mElementSize, "Expected template parameter be of same size as the underlying element");

	void* ptr = allocate();
	if (!ptr)
		throw std::bad_alloc();

	return ::new (ptr) T(std::forward<Args>(args)...);
}

template <typename T>
inline void MemoryPool::destroy(T* ptr) noexcept(std::is_nothrow_destructible<T>::value)
{
	ptr->~T();
	deallocate(ptr);
}

} // namespace PR