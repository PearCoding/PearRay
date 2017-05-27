#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>

/**
 * Available random algorithms
 * 0 - STL
 * 1 - Mult based
 * 2 - Xorshift RNGs (George Marsaglia) [Default]
 */

#ifndef PR_RANDOM_ALGORITHM
#define PR_RANDOM_ALGORITHM (2)
#endif

#if PR_RANDOM_ALGORITHM == 0
#include <random>
#endif

namespace PR {
class PR_LIB_INLINE Random {
private:
#if PR_RANDOM_ALGORITHM == 0
	std::default_random_engine mGenerator;
	std::uniform_real_distribution<float> mDistributionFloat;
	std::uniform_real_distribution<double> mDistributionDouble;
	std::uniform_int_distribution<uint32> mDistributionUInt32;
	std::uniform_int_distribution<uint64> mDistributionUInt64;
#elif PR_RANDOM_ALGORITHM == 1
	uint64 mState;
	uint64 mInc;
	uint64 mSeed;

	static constexpr uint64 MULT = 0x5851f42d4c957f2dULL; //6364136223846793005ULL
#else
	uint64 mState;
#endif
public:
	inline explicit Random(uint64 seed)
		:
#if PR_RANDOM_ALGORITHM == 0
		mGenerator(seed)
		, mDistributionFloat(0.0f, 1.0f)
		, mDistributionDouble(0.0, 1.0)
		, mDistributionUInt32()
		, mDistributionUInt64()
#elif PR_RANDOM_ALGORITHM == 1
		mState(0x64326ae2f48fe6dbULL)
		, mInc(0xf13e39cbe9a35bdbULL)
		, mSeed(seed)
#else
		mState(seed)
#endif
	{
	}

	inline uint32 get32()
	{
#if PR_RANDOM_ALGORITHM == 0
		return mDistributionUInt32(mGenerator);
#elif PR_RANDOM_ALGORITHM == 1
		uint64 oldstate = mState;

		// Advance internal state
		mState = oldstate * MULT + mInc;

		// Calculate output function (XSH RR), uses old state for max ILP
		uint32 xorshifted = (uint32)(((oldstate >> 18u) ^ oldstate) >> 27u);
		uint32 rot		  = (uint32)(oldstate >> 59u);

		return ((xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31))) ^ (uint32)mSeed;
#else
		return static_cast<uint32>(get64() & 0xFFFFFFFF);
#endif
	}

	inline uint32 get32(uint32 start, uint32 end)
	{
#if PR_RANDOM_ALGORITHM == 0
		return std::uniform_int_distribution<uint32>(start, end)(mGenerator);
#else
		return (get32() % (end - start)) + start;
#endif
	}

	inline uint64 get64()
	{
#if PR_RANDOM_ALGORITHM == 0
		return mDistributionUInt64(mGenerator);
#elif PR_RANDOM_ALGORITHM == 1
		return (((uint64)get32()) << 32) | (uint64)get32();
#else
		mState ^= (mState << 13);
		mState ^= (mState >> 7);
		return (mState ^= (mState << 17));
#endif
	}

	inline uint64 get64(uint64 start, uint64 end)
	{
#if PR_RANDOM_ALGORITHM == 0
		return std::uniform_int_distribution<uint64>(start, end)(mGenerator);
#else
		return (get64() % (end - start)) + start;
#endif
	}

	inline float getFloat() //[0, 1]
	{
#if PR_RANDOM_ALGORITHM == 0
		return mDistributionFloat(mGenerator);
#else
		//return get32() * 2.328306436538696e-10f;
		return get32() / static_cast<float>(std::numeric_limits<uint32>::max());
#endif
	}

	inline double getDouble() //[0, 1]
	{
#if PR_RANDOM_ALGORITHM == 0
		return mDistributionDouble(mGenerator);
#else
		return get64() / static_cast<double>(std::numeric_limits<uint64>::max());
#endif
	}

	inline Eigen::Vector2f get2D()
	{
		return Eigen::Vector2f(getFloat(), getFloat());
	}

	inline Eigen::Vector3f get3D()
	{
		return Eigen::Vector3f(getFloat(), getFloat(), getFloat());
	}

	inline Eigen::Vector4f get4D()
	{
		return Eigen::Vector4f(getFloat(), getFloat(), getFloat(), getFloat());
	}
};
}
