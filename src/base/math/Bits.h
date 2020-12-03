#pragma once

#include "PR_Config.h"

namespace PR {

/* Undefined if v == 0 */
inline uint32 clz(uint32 v)
{
#ifdef PR_CC_GNU
	return __builtin_clz(v);
#elif defined(PR_CC_MSC)
	return __lzcnt(v);
#else
	uint32 r = 0;
	while (v >>= 1)
		r++;
	return r;
#endif
}

/// Most Significant Bit
/// Sets the most significant bit bounded by v
inline uint32 msb(uint32 v)
{
	return 1 << (32 - clz(v));
}

/// Value Effect Mask
/// Set all bits till the left most bit of v
inline uint32 vem(uint32 v)
{
	uint32 max = msb(v);
	return max | (max - 1);
}

inline uint64 pack_even(uint64 x)
{
	x = (x | (x << 16)) & 0x0000FFFF0000FFFF;
	x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
	x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
	x = (x | (x << 2)) & 0x3333333333333333;
	x = (x | (x << 1)) & 0x5555555555555555;
	return x;
}

inline uint64 pack_k3(uint64 x)
{
	x = (x | (x << 32)) & 0x001F00000000FFFF;
	x = (x | (x << 16)) & 0x001F0000FF0000FF;
	x = (x | (x << 8)) & 0x100F00F00F00F00F;
	x = (x | (x << 4)) & 0x10C30C30C30C30C3;
	x = (x | (x << 2)) & 0x1249249249249249;
	return x;
}

/// Compose two positions into morton code. Max number for each component is 0xFFFFFFFF
inline uint64 xy_2_morton(uint32 x, uint32 y)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	return _pdep_u64(x, 0x5555555555555555) | _pdep_u64(y, 0xAAAAAAAAAAAAAAAA);
#else
	return pack_even(x) | (pack_even(y) << 1);
#endif
}

/// Compose three positions into morton code. Max number for each component is 0x1FFFFF
inline uint64 xyz_2_morton(uint32 x, uint32 y, uint32 z)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	return _pdep_u64(x, 0x1249249249249249) | _pdep_u64(y, 0x2492492492492492) | _pdep_u64(z, 0x4924924924924924);
#else
	return pack_k3(x) | (pack_k3(y) << 1) | (pack_k3(z) << 2);
#endif
}

/// extract even bits
inline uint32 unpack_even(uint64 x)
{
	x = x & 0x5555555555555555;
	x = (x | (x >> 1)) & 0x3333333333333333;
	x = (x | (x >> 2)) & 0x0F0F0F0F0F0F0F0F;
	x = (x | (x >> 4)) & 0x00FF00FF00FF00FF;
	x = (x | (x >> 8)) & 0x0000FFFF0000FFFF;
	x = (x | (x >> 16)) & 0x00000000FFFFFFFF;
	return (uint32)x;
}

inline uint32 unpack_k3(uint64 x)
{
	x = x & 0x1249249249249249;
	x = (x | (x >> 2)) & 0x10C30C30C30C30C3;
	x = (x | (x >> 4)) & 0x100F00F00F00F00F;
	x = (x | (x >> 8)) & 0x001F0000FF0000FF;
	x = (x | (x >> 16)) & 0x001F00000000FFFF;
	x = (x | (x >> 32)) & 0x00000000001FFFFF;
	return (uint32)x;
}

/// Decompose morton code into two positions
inline void morton_2_xy(uint64 d, uint32& x, uint32& y)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	x = _pext_u64(d, 0x5555555555555555);
	y = _pext_u64(d, 0xAAAAAAAAAAAAAAAA);
#else
	x = unpack_even(d);
	y = unpack_even(d >> 1);
#endif
}

inline void morton_2_xyz(uint64 d, uint32& x, uint32& y, uint32& z)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	x = _pext_u64(d, 0x1249249249249249);
	y = _pext_u64(d, 0x2492492492492492);
	z = _pext_u64(d, 0x4924924924924924);
#else
	x = unpack_k3(d);
	y = unpack_k3(d >> 1);
	z = unpack_k3(d >> 2);
#endif
}

} // namespace PR