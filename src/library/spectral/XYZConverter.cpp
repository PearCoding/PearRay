#include "XYZConverter.h"
#include "Spectrum.h"

#include "PearMath.h"

#define PR_XYZ_LINEAR_INTERP

namespace PR
{
	/* Source: https://upload.wikimedia.org/wikipedia/commons/8/8f/CIE_1931_XYZ_Color_Matching_Functions.svg */
	float NM_TO_XYZ[Spectrum::SAMPLING_COUNT*3] = 
	{/* X		Y		Z */
		0,		0,		0,		// 380 nm
		0.005f,	0,		0.05f,	// 390 nm [X Begin] [Z Begin]
		0.015f,	0,		0.08f,	// 400 nm 
		0.05f,	0,		0.18f,	// 410 nm 
		0.11f,	0,		0.55f,	// 420 nm
		0.29f,	0.005f,	1.22f,	// 430 nm [Y Begin]
		0.35f,	0.045f,	1.72f,	// 440 nm 
		0.33f,	0.055f,	1.725f,	// 450 nm 
		0.3f,	0.075f,	1.68f,	// 460 nm
		0.2f,	0.095f,	1.325f,	// 470 nm
		0.1f,	0.110f,	0.875f,	// 480 nm
		0.05f,	0.2f,	0.45f,	// 490 nm
		0.005f,	0.3f,	0.25f,	// 500 nm
		0.005f,	0.5f,	0.15f,	// 510 nm
		0.09f,	0.73f,	0.1f,	// 520 nm
		0.15f,	0.885f,	0.05f,	// 530 nm
		0.285f,	0.975f,	0.025f,	// 540 nm [Z End]
		0.43f,	1.0f,	0,		// 550 nm
		0.56f,	1.0f,	0,		// 560 nm
		0.74f,	0.96f,	0,		// 570 nm
		0.9f,	0.88f,	0,		// 580 nm
		1.025f,	0.725f,	0,		// 590 nm
		1.05f,	0.6f,	0,		// 600 nm
		0.98f,	0.49f,	0,		// 610 nm
		0.905f,	0.35f,	0,		// 620 nm
		0.65f,	0.25f,	0,		// 630 nm
		0.455f,	0.2f,	0,		// 640 nm
		0.245f,	0.11f,	0,		// 650 nm
		0.185f,	0.09f,	0,		// 660 nm
		0.1f,	0.05f,	0,		// 670 nm
		0.05f,	0.025f,	0,		// 680 nm [Y End]
		0.025f,	0,		0,		// 690 nm
		0.005f,	0,		0,		// 700 nm [X End]
		0,		0,		0,		// 710 nm
		0,		0,		0,		// 720 nm
		0,		0,		0,		// 730 nm
		0,		0,		0,		// 740 nm
		0,		0,		0,		// 750 nm
		0,		0,		0,		// 760 nm
		0,		0,		0,		// 770 nm
		0,		0,		0,		// 780 nm
	};

	void XYZConverter::convertXYZ(const Spectrum& s, float &X, float &Y, float &Z)
	{
		X = 0;
		Y = 0;
		Z = 0;

		for (uint32 i = 1; i < Spectrum::SAMPLING_COUNT - 7; ++i)
		{
			float val1 = s.value(i);

#ifdef PR_XYZ_LINEAR_INTERP
			float val2 = s.value(i + 1);

			X += val1 * NM_TO_XYZ[3 * i] + val2 * NM_TO_XYZ[3 * (i + 1)];
			Y += val1 * NM_TO_XYZ[3 * i + 1] + val2 * NM_TO_XYZ[3 * (i + 1) + 1];
			Z += val1 * NM_TO_XYZ[3 * i + 2] + val2 * NM_TO_XYZ[3 * (i + 1) + 2];
#else
			X += val1 * NM_TO_XYZ[3 * i];
			Y += val1 * NM_TO_XYZ[3 * i + 1];
			Z += val1 * NM_TO_XYZ[3 * i + 2];
#endif
		}

		//std::cout << x << " " << y << " " << z << std::endl;
		constexpr float scale =
			((float)Spectrum::WAVELENGTH_END - Spectrum::WAVELENGTH_START) / (106.856895f * Spectrum::SAMPLING_COUNT);

		X *= scale;
		Y *= scale;
		Z *= scale;

#ifdef PR_XYZ_LINEAR_INTERP
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;
#endif
	}

	void XYZConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		float X, Y, Z;
		convertXYZ(s, X, Y, Z);

		/*x = X;
		y = Y;
		z = Z;*/
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