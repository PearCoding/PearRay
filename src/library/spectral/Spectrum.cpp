#include "Spectrum.h"

namespace PR
{
	Spectrum::Spectrum()
	{
		memset(mValues, 0, sizeof(float)*SAMPLING_COUNT);
	}

	Spectrum::~Spectrum()
	{

	}

	void Spectrum::setValue(uint32 index, float v)
	{
		mValues[index] = v;
	}

	float Spectrum::value(uint32 index) const
	{
		return mValues[index];
	}

	void Spectrum::setValueAtWavelength(float wavelength, float value)
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		{
			return;
		}

		uint32 i = (wavelength - WAVELENGTH_START) / WAVELENGTH_STEP;
		mValues[i] = value;
	}

	float Spectrum::approx(float wavelength, InterpolationType interpolation) const
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		{
			return -1;
		}

		float st = wavelength - WAVELENGTH_START;
		uint32 c = st / WAVELENGTH_STEP;
		uint32 m = fmodf(st, WAVELENGTH_STEP);

		if (st < 0 || c > SAMPLING_COUNT)
		{
			return 0;
		}

		if(interpolation == IT_Const)
		{ 
			if (m >= WAVELENGTH_STEP / 2 && c != SAMPLING_COUNT)
			{
				return mValues[c+1];
			}
			else
			{
				return mValues[c];
			}
		}
		else
		{
			if (c == SAMPLING_COUNT)
			{
				return mValues[c];
			}
			else
			{
				float t = m / (float)WAVELENGTH_STEP;
				return mValues[c] * (1 - t) + mValues[c + 1] * t;
			}
		}
	}

	float Spectrum::max() const
	{
		float h = 0;

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h < fabsf(mValues[i]))
			{
				h = fabsf(mValues[i]);
			}
		}

		return h;
	}

	float Spectrum::min() const
	{
		float h = std::numeric_limits<float>::max();

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h > fabsf(mValues[i]))
			{
				h = fabsf(mValues[i]);
			}
		}

		return h;
	}

	float Spectrum::avg() const
	{
		float h = 0;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			h += mValues[i];
		}

		return h / SAMPLING_COUNT;
	}

	void Spectrum::normalize()
	{
		float h = max();

		if (h != 0)
		{
			for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			{
				mValues[i] /= h;
			}
		}
	}
}