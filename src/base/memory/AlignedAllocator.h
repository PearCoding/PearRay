#pragma once

#include <boost/align/aligned_allocator.hpp>

namespace PR {
template <typename T, size_t N>
using AlignedAllocator = boost::alignment::aligned_allocator<T, N>;
} // namespace PR