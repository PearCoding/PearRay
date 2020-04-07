// IWYU pragma: private, include "memory/MemoryStack.h"
namespace PR {
inline MemoryStack::MemoryStack(size_t maxSize)
	: mMemory(maxSize)
{
	PR_ASSERT(maxSize >= sizeof(marker_t), "Expected at least memory for one marker");
}

inline MemoryStack::~MemoryStack()
{
}

inline void* MemoryStack::allocate(size_t size, size_t alignment) noexcept
{
	PR_ASSERT(size > 0, "Expected a valid memory size");
	PR_ASSERT(alignment > 0 && (alignment & (alignment - 1)) == 0, "Expected alignment greater 0 and of power 2");

	if (usedMemory() + size > maxMemory())
		return nullptr;

	void* ptr	 = &mMemory[mUsedMemory];
	size_t space = size + alignment;
	std::align(alignment, size, ptr, space);

	if (!ptr)
		return nullptr;

	size_t alignmentOff = size + alignment - (ptrdiff_t)space;
	size_t realSpace	= size + alignmentOff;

	if (usedMemory() + realSpace > maxMemory())
		return nullptr;

	mUsedMemory += realSpace;
	return ptr;
}

inline void MemoryStack::mark()
{
	marker_t markerValue = mUsedMemory;

	marker_t* marker = allocateMarker();
	*marker			 = mLastMarker;
	mLastMarker		 = markerValue;
	++mMarkerCount;
}

inline void MemoryStack::freeUntilMarker() noexcept
{
	if (mMarkerCount == 0) {
		freeAll();
	} else {
		mUsedMemory = mLastMarker;
		mLastMarker = mMemory[mLastMarker];
		--mMarkerCount;
	}
}

inline void MemoryStack::freeAll() noexcept
{
	mUsedMemory	 = 0;
	mLastMarker	 = 0;
	mMarkerCount = 0;
}

template <typename T, class... Args>
inline T* MemoryStack::create(Args... args)
{
	void* ptr = allocate(sizeof(T), alignof(T));
	if (!ptr)
		throw std::bad_alloc();

	return ::new (ptr) T(std::forward<Args>(args)...);
}

inline MemoryStack::marker_t* MemoryStack::allocateMarker()
{
	void* ptr = allocate(sizeof(marker_t));
	if (!ptr)
		throw std::bad_alloc();

	return reinterpret_cast<marker_t*>(ptr);
}

} // namespace PR