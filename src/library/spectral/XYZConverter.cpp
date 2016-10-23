#include "XYZConverter.h"
#include "Spectrum.h"

#include "PearMath.h"

//#define PR_XYZ_LINEAR_INTERP

namespace PR
{
#ifndef PR_NO_SPECTRAL
# define SAMPLING_COUNT (Spectrum::SAMPLING_COUNT)
# define constant const
# include "cl/xyztable.cl"
# undef SAMPLING_COUNT
# undef constant
	constexpr float N = 21.371327f * Spectrum::ILL_SCALE;

#endif

	void XYZConverter::convertXYZ(const Spectrum& s, float &X, float &Y, float &Z)
	{
#ifndef PR_NO_SPECTRAL
		X = 0;
		Y = 0;
		Z = 0;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float val1 = s.value(i);

# ifdef PR_XYZ_LINEAR_INTERP
			if (i < Spectrum::SAMPLING_COUNT - 1)
			{
				float val2 = s.value(i + 1);

				X += val1 * NM_TO_X[i] + val2 * NM_TO_X[i + 1];
				Y += val1 * NM_TO_Y[i] + val2 * NM_TO_Y[i + 1];
				Z += val1 * NM_TO_Z[i] + val2 * NM_TO_Z[i + 1];
			}
			else
			{
# endif
				X += val1 * NM_TO_X[i];
				Y += val1 * NM_TO_Y[i];
				Z += val1 * NM_TO_Z[i];
# ifdef PR_XYZ_LINEAR_INTERP
			}
# endif
		}

		X *= Spectrum::ILL_SCALE;
		Y *= Spectrum::ILL_SCALE;
		Z *= Spectrum::ILL_SCALE;

		if (!s.isEmissive())
		{
			X /= N;
			Y /= N;
			Z /= N;
		}

# ifdef PR_XYZ_LINEAR_INTERP
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;
# endif
#else// PR_NO_SPECTRAL
		X = 0.4124f * s.value(0) + 0.3576f * s.value(1) + 0.1805f * s.value(2);
		Y = 0.2126f * s.value(0) + 0.7152f * s.value(1) + 0.0722f * s.value(2);
		Z = 0.0193f * s.value(0) + 0.1192f * s.value(1) + 0.9505f * s.value(2);
#endif
	}

	void XYZConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		float X, Y, Z;
		convertXYZ(s, X, Y, Z);

		float m = X + Y + Z;
		if (m != 0)
		{
			x = X / m;
			y = Y / m;
			z = 1 - x - y;
		}
		else
		{
			x = 0; y = 0; z = 1;
		}
	}
}