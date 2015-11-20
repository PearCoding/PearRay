#include "Spectrum.h"

namespace PR
{
	float Spectrum::approx(float wavelength, InterpolationType interpolation) const
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		{
			return -1;
		}

		float st = wavelength - WAVELENGTH_START;
		uint32 c = (uint32)(st / WAVELENGTH_STEP);
		uint32 m = (uint32)fmodf(st, (float)WAVELENGTH_STEP);

		if (st < 0 || c >= SAMPLING_COUNT)
		{
			return 0;
		}

		if(interpolation == IT_Const)
		{ 
			if (m >= WAVELENGTH_STEP / 2 && c < SAMPLING_COUNT - 1)
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
			if (c >= SAMPLING_COUNT - 1)
			{
				return mValues[SAMPLING_COUNT - 1];
			}
			else
			{
				float t = m / (float)WAVELENGTH_STEP;
				return mValues[c] * (1 - t) + mValues[c + 1] * t;
			}
		}
	}
}