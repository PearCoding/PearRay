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

/// Compose two positions into morton code
inline uint64 xy_2_morton(uint32 x, uint32 y)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	return _pdep_u32(x, 0x55555555) | _pdep_u32(y, 0xAAAAAAAA);
#else
	x = (x | (x << 16)) & 0x0000FFFF0000FFFF;
	x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
	x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
	x = (x | (x << 2)) & 0x3333333333333333;
	x = (x | (x << 1)) & 0x5555555555555555;

	y = (y | (y << 16)) & 0x0000FFFF0000FFFF;
	y = (y | (y << 8)) & 0x00FF00FF00FF00FF;
	y = (y | (y << 4)) & 0x0F0F0F0F0F0F0F0F;
	y = (y | (y << 2)) & 0x3333333333333333;
	y = (y | (y << 1)) & 0x5555555555555555;

	return x | (y << 1);
#endif
}

/// extract even bits
inline uint32 extract_even(uint64 x)
{
	x = x & 0x5555555555555555;
	x = (x | (x >> 1)) & 0x3333333333333333;
	x = (x | (x >> 2)) & 0x0F0F0F0F0F0F0F0F;
	x = (x | (x >> 4)) & 0x00FF00FF00FF00FF;
	x = (x | (x >> 8)) & 0x0000FFFF0000FFFF;
	x = (x | (x >> 16)) & 0x00000000FFFFFFFF;
	return (uint32)x;
}

/// Decompose morton code into two positions
inline void morton_2_xy(uint64 d, uint32& x, uint32& y)
{
#ifdef PR_HAS_HW_FEATURE_BMI2
	x = _pext_u64(d, 0x5555555555555555);
	y = _pext_u64(d, 0xAAAAAAAAAAAAAAAA);
#else
	x = extract_even(d);
	y = extract_even(d >> 1);
#endif
}

} // namespace PR