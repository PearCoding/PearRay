#include "Spectrum.h"

namespace PR
{
#ifndef PR_NO_SPECTRAL
# define SAMPLING_COUNT (Spectrum::SAMPLING_COUNT)
# define constant const
# include "cl/xyztable.cl"
# undef SAMPLING_COUNT
# undef constant

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
#endif//PR_NO_SPECTRAL

	constexpr float CANDELA = 683.002f;
	void Spectrum::weightPhotometric()
	{
#ifndef PR_NO_SPECTRAL
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] *= NM_TO_Y[i] * CANDELA;
#endif
	}

	float Spectrum::luminousFlux() const
	{
#ifndef PR_NO_SPECTRAL
		float flux = 0;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			flux += mValues[i] * NM_TO_Y[i];
		
		return flux * ILL_SCALE * CANDELA;
#else
		return mValues[0] * 0.2126f + mValues[1] * 0.7152f + mValues[2] * 0.0722f;
#endif
	}

	// Has to be in double!
	inline long double blackbody_eq(long double temp, long double lambda_nm)
	{
		constexpr long double c = 299792458l;
		constexpr long double h = 6.626070040e-34l;
		constexpr long double kb = 1.38064852e-23l;
		
		constexpr long double c1 = 2*h*c*c;
		constexpr long double c2 = h*c/kb;

		const long double lambda5 = lambda_nm * (lambda_nm * lambda_nm) * (lambda_nm * lambda_nm);
		return (c1 / lambda5) / (std::exp(c2 / (lambda_nm * temp)) - 1);
	}

	Spectrum Spectrum::fromBlackbody(float temp)
	{
		Spectrum spec;
#ifndef PR_NO_SPECTRAL
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			long double lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9l;
			spec.mValues[i] = static_cast<float>(blackbody_eq(temp, lambda));
		}
#else
		spec.mValues[0] = static_cast<float>(blackbody_eq(temp, 640*1e-9l));
		spec.mValues[1] = static_cast<float>(blackbody_eq(temp, 550*1e-9l));
		spec.mValues[2] = static_cast<float>(blackbody_eq(temp, 430*1e-9l));
#endif

		return spec;
	}
}