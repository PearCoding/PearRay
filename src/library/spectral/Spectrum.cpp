#include "Spectrum.h"

#include "PearMath.h"

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

	// Has to be in double!
	inline double blackbody_eq(double temp, double lambda)
	{
		constexpr double c = 299792458;
		constexpr double h = 6.62606957e-34f;
		constexpr double kb = 1.3806488e-23f;

		const double lambda5 = lambda * (lambda * lambda) * (lambda * lambda);
		return (2 * h * c * c) / (lambda5 * (std::exp((h * c) / (lambda * kb * temp)) - 1));
	}

	Spectrum Spectrum::fromBlackbody(float temp)
	{
		Spectrum spec;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9f;
			spec.mValues[i] = static_cast<float>(blackbody_eq(temp, lambda));
		}

		return spec;
	}

	Spectrum Spectrum::fromBlackbodyNorm(float temp)
	{
		const double maxLambda = 2.897772917e-3f / temp;
		const double norm = 1/blackbody_eq(temp, maxLambda);

		Spectrum spec;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9f;
			spec.mValues[i] = static_cast<float>(blackbody_eq(temp, lambda) * norm);
		}

		return spec;
	}

	Spectrum Spectrum::fromBlackbodyHemi(float temp)
	{
		return fromBlackbody(temp) * PM_2_PI_F;
	}

	Spectrum Spectrum::fromBlackbodySphere(float temp)
	{
		return fromBlackbody(temp) * PM_4_PI_F;
	}
}