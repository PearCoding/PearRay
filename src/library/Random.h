#pragma once

#include "Config.h"

namespace PR
{
	class PR_LIB_INLINE Random
	{
	private:
		static const uint32_t N  = 624;
		static const uint32_t M  = 397;
		static const uint32_t HI = 0x80000000;
		static const uint32_t LO = 0x7fffffff;

		const uint32_t A[2] = { 0, 0x9908b0df };
		uint32_t seed = 126943UL;//5489UL;

		uint32_t index = N + 1;
		uint32_t y[N];

	public:
		static const uint32 MAX = 0xFFFFFFFF;
		Random(uint32 s = 126943UL) :
			seed(s)
		{
		}

		inline uint32 generate() {
			uint32_t  e;

			if (index > N)
			{
				uint32_t i;
				/* Init y with seed */
				y[0] = seed;

				for (i = 1; i < N; ++i)
				{
					y[i] = (1812433253UL * (y[i - 1] ^ (y[i - 1] >> 30)) + i);
					/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
					/* In the previous versions, MSBs of the seed affect   */
					/* only MSBs of the array mt[].                        */
					/* 2002/01/09 modified by Makoto Matsumoto             */
				}
			}

			if (index >= N)
			{
				uint32_t i;
				/* Calculate new vector */
				uint32_t h;

				for (i = 0; i < N - M; ++i) {
					h = (y[i] & HI) | (y[i + 1] & LO);
					y[i] = y[i + M] ^ (h >> 1) ^ A[h & 1];
				}

				for (; i < N - 1; ++i) {
					h = (y[i] & HI) | (y[i + 1] & LO);
					y[i] = y[i + (M - N)] ^ (h >> 1) ^ A[h & 1];
				}

				h = (y[N - 1] & HI) | (y[0] & LO);
				y[N - 1] = y[M - 1] ^ (h >> 1) ^ A[h & 1];
				index = 0;
			}

			e = y[index++];

			/* Tempering */
			e ^= (e >> 11);
			e ^= (e << 7) & 0x9d2c5680;
			e ^= (e << 15) & 0xefc60000;
			e ^= (e >> 18);

			return e;
		}
	};
}