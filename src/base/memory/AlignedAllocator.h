#pragma once

#include "PR_Config.h"

#include <cstdlib>
#include <memory>

namespace PR {
template <typename T, size_t Alignment>
class AlignedAllocator {
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef void* void_pointer;
	typedef const void* const_void_pointer;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T& reference;
	typedef const T& const_reference;

	template <typename U>
	struct rebind {
		// types
		typedef AlignedAllocator<U, Alignment> other;
	};

	inline AlignedAllocator() = default;
	template <typename U, size_t N2>
	inline AlignedAllocator(const AlignedAllocator<U, N2>&) noexcept {}

	[[nodiscard]] inline T* allocate(std::size_t n)
	{
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
			throw std::bad_alloc();

#ifdef PR_OS_WINDOWS
		if (auto p = static_cast<T*>(_aligned_malloc(n * sizeof(T), Alignment)))
			return p;
#else
		if (auto p = static_cast<T*>(std::aligned_alloc(Alignment, n * sizeof(T))))
			return p;
#endif

		throw std::bad_alloc();
	}

	inline void deallocate(T* ptr, std::size_t)
	{
		std::free(ptr);
	}
};

template <class T, size_t N1, class U, size_t N2>
bool operator==(const AlignedAllocator<T, N1>&, const AlignedAllocator<U, N2>&) { return true; }
template <class T, size_t N1, class U, size_t N2>
bool operator!=(const AlignedAllocator<T, N1>&, const AlignedAllocator<U, N2>&) { return false; }
} // namespace PR