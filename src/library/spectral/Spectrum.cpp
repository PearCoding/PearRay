#include "Spectrum.h"

namespace PR
{
	float Spectrum::approx(float wavelength, InterpolationType interpolation) const
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
			return -1;

		float st = wavelength - WAVELENGTH_START;
		uint32 c = (uint32)(st / WAVELENGTH_STEP);
		uint32 m = (uint32)fmodf(st, (float)WAVELENGTH_STEP);

		if (st < 0 || c >= SAMPLING_COUNT)
			return 0;

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

	// Has to be in double!
	inline long double blackbody_eq(long double temp, long double lambda_nm)
	{
		constexpr long double c = 299792458l;
		constexpr long double h = 6.62606957e-34l;
		constexpr long double kb = 1.3806488e-23l;
		
		constexpr long double c1 = 3.741771525e-16l;// PM_2_PI * h * c * c
		constexpr long double c2 = 1.43877696e-2l;// h*c/kb

		const long double lambda5 = lambda_nm * (lambda_nm * lambda_nm) * (lambda_nm * lambda_nm);
		return c1 / (lambda5 * (std::exp(c2 / (lambda_nm * temp)) - 1));
	}

	Spectrum Spectrum::fromBlackbody(float temp)
	{
		Spectrum spec;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			long double lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9l;
			spec.mValues[i] = static_cast<float>(blackbody_eq(temp, lambda));
		}

		return spec;
	}

	Spectrum Spectrum::fromBlackbodyNorm(float temp)
	{
		const long double maxLambda = 2.897772917e-3l / temp;
		const long double norm = 1/blackbody_eq(temp, maxLambda);

		Spectrum spec;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			long double lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9l;
			spec.mValues[i] = static_cast<float>(blackbody_eq(temp, lambda) * norm);
		}

		return spec;
	}
}