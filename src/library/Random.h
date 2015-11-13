#pragma once

#include "Config.h"

#include <math.h>

namespace PR
{
	class PR_LIB_INLINE Random
	{
	private:
		uint64_t mState;
		uint64_t mInc;

	public:
		Random(uint64_t s = 126943) :
			mState(s), mInc(static_cast<uint64_t>((uintptr_t)this))//Address
		{
		}

		inline uint32_t get32()
		{
			uint64_t oldstate = mState;

			// Advance internal state
			mState = oldstate * (uint64_t)6364136223846793005ULL + (mInc | 1);

			// Calculate output function (XSH RR), uses old state for max ILP
			uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
			uint32_t rot = (uint32_t)(oldstate >> 59u);

			return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
		}

		inline uint32_t get32(uint32_t start, uint32_t end)
		{
			return get32() % (end - start) + start;
		}

		inline uint64_t get64()
		{
			return  (((uint64_t)get32()) << 32) | (uint64_t)get32();
		}

		inline uint64_t get64(uint64_t start, uint64_t end)
		{
			return get64() % (end - start) + start;
		}

		inline float getFloat()//[0, 1)
		{
			return std::fmin(0.9999999403953552f, get32() * 2.3283064365386963e-10f);
		}

		inline double getDouble()//[0, 1)
		{
			return (double)getFloat();//TODO
		}
	};
}