#include "XYZConverter.h"

//#define PR_XYZ_LINEAR_INTERP

namespace PR
{
# define SAMPLING_COUNT (Spectrum::SAMPLING_COUNT)
# define constant const
# include "cl/xyztable.cl"
# undef SAMPLING_COUNT
# undef constant
	constexpr float Y_SUM = 21.371327f;
	constexpr float N = Y_SUM * Spectrum::ILL_SCALE;

	void XYZConverter::convertXYZ(const float* src, float &X, float &Y, float &Z)
	{
		X = 0;
		Y = 0;
		Z = 0;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float val1 = src[i];

# ifdef PR_XYZ_LINEAR_INTERP
			if (i < Spectrum::SAMPLING_COUNT - 1)
			{
				float val2 = src[i + 1];

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

		X /= Y_SUM;
		Y /= Y_SUM;
		Z /= Y_SUM;

# ifdef PR_XYZ_LINEAR_INTERP
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;
# endif
	}

	void XYZConverter::convert(const float* src, float &x, float &y, float &z)
	{
		float X, Y, Z;
		convertXYZ(src, X, Y, Z);

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
