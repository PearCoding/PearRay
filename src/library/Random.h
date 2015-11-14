#pragma once

#include "Config.h"

#ifdef PR_USE_STANDARD_RANDOM
# include <random>
#endif

namespace PR
{
	class PR_LIB_INLINE Random
	{
	private:
#ifdef PR_USE_STANDARD_RANDOM
		std::default_random_engine mGenerator;
		std::uniform_real_distribution<float> mDistributionFloat;
		std::uniform_real_distribution<double> mDistributionDouble;
		std::uniform_int_distribution<uint32> mDistributionUInt32;
		std::uniform_int_distribution<uint64> mDistributionUInt64;
#else
		uint64_t mState;
		uint64_t mInc;

		static constexpr uint64_t MULT = 0x5851f42d4c957f2dULL;//6364136223846793005ULL
#endif
	public:
		inline explicit Random(uint64_t s = 0x64326ae2f48fe6dbULL) :
#ifdef PR_USE_STANDARD_RANDOM
			mGenerator(s), mDistributionFloat(0.0f, 1.0f), mDistributionDouble(0.0,1.0),
			mDistributionUInt32(), mDistributionUInt64()
#else
			mState(s), mInc(0xf13e39cbe9a35bdbULL)
#endif
		{
		}

		inline uint32_t get32()
		{
#ifdef PR_USE_STANDARD_RANDOM
			return mDistributionUInt32(mGenerator);
#else
			uint64_t oldstate = mState;

			// Advance internal state
			mState = oldstate * MULT + mInc;

			// Calculate output function (XSH RR), uses old state for max ILP
			uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
			uint32_t rot = (uint32_t)(oldstate >> 59u);

			return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
#endif
		}

		inline uint32_t get32(uint32_t start, uint32_t end)
		{
#ifdef PR_USE_STANDARD_RANDOM
			return std::uniform_int_distribution<uint32>(start, end)(mGenerator);
#else
			return get32() % (end - start) + start;
#endif
		}

		inline uint64_t get64()
		{
#ifdef PR_USE_STANDARD_RANDOM
			return mDistributionUInt64(mGenerator);
#else
			return  (((uint64_t)get32()) << 32) | (uint64_t)get32();
#endif
		}

		inline uint64_t get64(uint64_t start, uint64_t end)
		{
#ifdef PR_USE_STANDARD_RANDOM
			return std::uniform_int_distribution<uint64>(start, end)(mGenerator);
#else
			return get64() % (end - start) + start;
#endif
		}

		inline float getFloat()//[0, 1]
		{
#ifdef PR_USE_STANDARD_RANDOM
			return mDistributionFloat(mGenerator);
#else
			// FIXME: Really that good?
			return get32() * 2.328306436538696e-10f;
#endif
		}

		inline double getDouble()//[0, 1]
		{
#ifdef PR_USE_STANDARD_RANDOM
			return mDistributionDouble(mGenerator);
#else
			return getFloat();//TODO
#endif
		}
	};
}