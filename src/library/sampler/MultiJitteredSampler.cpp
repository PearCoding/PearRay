#include "MultiJitteredSampler.h"
#include "Projection.h"

namespace PR
{
	MultiJitteredSampler::MultiJitteredSampler(Random& random, uint32 samples) :
		Sampler(), mRandom(random), mSamples(samples),
		mIndex(0)
	{
		mM = std::sqrt(samples);
		mN = (mSamples + mM - 1) / mM;
	}

	MultiJitteredSampler::~MultiJitteredSampler()
	{
	}

	float MultiJitteredSampler::generate1D()
	{
		uint32 p = mRandom.get32();
		uint32 s = permute(mIndex % mSamples, mSamples, p * 0xa511e9b3);
		float j = randfloat(mIndex, p * 0xa399d265);
		float r = (mIndex % mSamples + j) / mSamples;
		mIndex++;
		return r;
	}

	PM::vec2 MultiJitteredSampler::generate2D()
	{
		uint32 p = mRandom.get32();
		uint32 sx = permute(mIndex % mM, mM, p * 0xa511e9b3);
		uint32 sy = permute(mIndex / mM, mN, p * 0x63d83595);
		float jx = randfloat(mIndex, p * 0xa399d265);
		float jy = randfloat(mIndex, p * 0x711ad6a5);
		auto r = PM::pm_Set((mIndex % mM + (sy + jx) / mN) / mM,
			(mIndex / mM + (sx + jy) / mM) / mN);
		mIndex++;
		return r;
	}

	// Not really uniform!
	PM::vec3 MultiJitteredSampler::generate3D()
	{
		auto x = generate1D();
		auto yz = generate2D();
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
		i ^= i >> 10; i *= 0xb36534e5;
		i ^= i >> 12;
		i ^= i >> 21; i *= 0x93fc4795;
		i ^= 0xdf6e307f;
		i ^= i >> 17; i *= 1 | p >> 18;
		return i * (1.0f / 4294967808.0f);
	}

	void MultiJitteredSampler::reset()
	{
		mIndex = 0;
	}
}