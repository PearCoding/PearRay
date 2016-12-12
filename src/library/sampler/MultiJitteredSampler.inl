#pragma once

namespace PR
{
	float MultiJitteredSampler::generate1D(uint32 index)
	{
		//uint32 s = permute(index % mSamples, mSamples, p * 0xa399d265);

#ifndef PR_MJS_USE_RANDOM
		uint32 p = mRandom.get32();
		float j = randfloat(index % mSamples, p * 0x711ad6a5);
#else
		float j = mRandom.getFloat();
#endif

		float r = (index % mSamples + j) / mSamples;
		return r;
	}

	PM::vec2 MultiJitteredSampler::generate2D(uint32 index)
	{
		uint32 newIndex = index % mSamples;

		uint32 p = mRandom.get32();
		uint32 sx = permute(newIndex % m2D_X, m2D_X, p * 0xa511e9b3);
		uint32 sy = permute(newIndex / m2D_X, m2D_Y, p * 0x63d83595);

#ifndef PR_MJS_USE_RANDOM
		float jx = randfloat(newIndex, p * 0xa399d265);
		float jy = randfloat(newIndex, p * 0x711ad6a5);
#else
		float jx = mRandom.getFloat();
		float jy = mRandom.getFloat();
#endif

		auto r = PM::pm_Set((newIndex % m2D_X + (sy + jx) / m2D_Y) / m2D_X,
			(newIndex / m2D_X + (sx + jy) / m2D_X) / m2D_Y);
		return r;
	}

	// Not really uniform!
	PM::vec3 MultiJitteredSampler::generate3D(uint32 index)
	{
		auto x = generate1D(index);
		auto yz = generate2D(index);
		return PM::pm_Set(x, PM::pm_GetX(yz), PM::pm_GetY(yz));
	}

	uint32 MultiJitteredSampler::permute(uint32 i, uint32 l, uint32 p)
	{
		uint32 w = l - 1;

		if (w == 0)
		{
			return 0;
		}
		else if ((l & w) == 0)// Power of 2
		{
			i ^= p;
			i *= 0xe170893d;
			i ^= p >> 16;
			i ^= (i & w) >> 4;
			i ^= p >> 8;
			i *= 0x0929eb3f;
			i ^= p >> 23;
			i ^= (i & w) >> 1;
			i *= 1 | p >> 27;
			i *= 0x6935fa69;
			i ^= (i & w) >> 11;
			i *= 0x74dcb303;
			i ^= (i & w) >> 2;
			i *= 0x9e501cc3;
			i ^= (i & w) >> 2;
			i *= 0xc860a3df;
			i &= w;
			i ^= i >> 5;
			return (i + p) & w;
		}
		else
		{
			w |= w >> 1;
			w |= w >> 2;
			w |= w >> 4;
			w |= w >> 8;
			w |= w >> 16;
			do {
				i ^= p;
				i *= 0xe170893d;
				i ^= p >> 16;
				i ^= (i & w) >> 4;
				i ^= p >> 8;
				i *= 0x0929eb3f;
				i ^= p >> 23;
				i ^= (i & w) >> 1;
				i *= 1 | p >> 27;
				i *= 0x6935fa69;
				i ^= (i & w) >> 11;
				i *= 0x74dcb303;
				i ^= (i & w) >> 2;
				i *= 0x9e501cc3;
				i ^= (i & w) >> 2;
				i *= 0xc860a3df;
				i &= w;
				i ^= i >> 5;
			} while (i >= l);
			return (i + p) % l;
		}
	}

	float MultiJitteredSampler::randfloat(uint32 i, uint32 p)
	{
		i ^= p;
		i ^= i >> 17;
		i ^= i >> 10;
		i *= 0xb36534e5;
		i ^= i >> 12;
		i ^= i >> 21;
		i *= 0x93fc4795;
		i ^= 0xdf6e307f;
		i ^= i >> 17; 
		i *= 1 | p >> 18;
		return i * (1.0f / 4294967808.0f);
	}
}