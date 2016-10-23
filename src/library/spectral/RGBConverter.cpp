#include "RGBConverter.h"
#include "XYZConverter.h"

#include "PearMath.h"

namespace PR
{
	/* Source: https://en.wikipedia.org/wiki/File:CIE1931_RGBCMF.svg */
	//float NM_TO_RGB[Spectrum::SAMPLING_COUNT * 3] =
	//{/* Red		Green	Blue */
	//	0,		0,		0,		// 380 nm
	//	0,		0,		0,		// 390 nm
	//	0,		0,		0.01f,	// 400 nm [BLUE START]
	//	0,		0,		0.04f,	// 410 nm 
	//	0,		0,		0.105f,	// 420 nm
	//	0,		0,		0.230f,	// 430 nm
	//	-0.005f,0,		0.310f,	// 440 nm [RED START]
	//	-0.015f,0.01f,	0.310f,	// 450 nm [GREEN START]
	//	-0.03f,	0.015f,	0.285f,	// 460 nm
	//	-0.04f,	0.02f,	0.225f,	// 470 nm
	//	-0.05f,	0.04f,	0.145f,	// 480 nm
	//	-0.055f,0.055f,	0.085f,	// 490 nm
	//	-0.07f,	0.08f,	0.050f,	// 500 nm
	//	-0.1f,	0.13f,	0.030f,	// 510 nm
	//	-0.1f,	0.175f,	0.015f,	// 520 nm
	//	-0.08f,	0.205f,	0.005f,	// 530 nm [BLUE END]
	//	-0.035f,0.210f,	0,		// 540 nm
	//	0.02f,	0.205f,	0,		// 550 nm
	//	0.07f,	0.195f,	0,		// 560 nm
	//	0.16f,	0.165f,	0,		// 570 nm
	//	0.24f,	0.140f,	0,		// 580 nm
	//	0.3f,	0.105f,	0,		// 590 nm
	//	0.33f,	0.065f,	0,		// 600 nm
	//	0.34f,	0.04f,	0,		// 610 nm
	//	0.32f,	0.02f,	0,		// 620 nm
	//	0.23f,	0.01f,	0,		// 630 nm
	//	0.155f,	0.005f,	0,		// 640 nm [GREEN END]
	//	0.11f,	0,		0,		// 650 nm
	//	0.06f,	0,		0,		// 660 nm
	//	0.03f,	0,		0,		// 670 nm
	//	0.015f,	0,		0,		// 680 nm
	//	0.01f,	0,		0,		// 690 nm
	//	0.005f,	0,		0,		// 700 nm [RED END]
	//	0,		0,		0,		// 710 nm
	//	0,		0,		0,		// 720 nm
	//	0,		0,		0,		// 730 nm
	//	0,		0,		0,		// 740 nm
	//	0,		0,		0,		// 750 nm
	//	0,		0,		0,		// 760 nm
	//	0,		0,		0,		// 770 nm
	//	0,		0,		0,		// 780 nm
	//};

	void RGBConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
#ifndef PR_NO_SPECTRAL
		float X, Y, Z;
		XYZConverter::convertXYZ(s, X, Y, Z);

		// To D65
		//X *= 0.95047f;
		//Y *= 1;
		//Z *= 1.08883f;

		// To D65 [Bradford]
		float X2 = 0.9531874f*X - 0.0265906f*Y + 0.0238731f*Z;
		float Y2 = -0.0382467f*X + 1.0288406f*Y + 0.0094060f*Z;
		float Z2 = 0.0026068f*X - 0.0030332f*Y + 1.0892565f*Z;

		// To D50
		//X *= 0.9642200f;
		//Y *= 1;
		//Z *= 0.8252100f;

		x = 3.240479f * X2 - 1.537150f * Y2 - 0.498535f * Z2;
		y = -0.969256f * X2 + 1.875991f * Y2 + 0.041556f * Z2;
		z = 0.055648f * X2 - 0.204043f * Y2 + 1.057311f * Z2;

		/*x = 3.2406f * X2 - 1.5372f * Y2 - 0.4986f * Z2;
		y = -0.9689f * X2 + 1.8758f * Y2 + 0.0415f * Z2;
		z = 0.0557f * X2 - 0.2040f * Y2 + 1.0570f * Z2;*/

		/*x =  0.025187164190036f * X - 0.011947757372197f * Y - 0.003874948336929f * Z;
		y = -0.009570175410578f * X + 0.018523039390656f * Y + 0.000410308795123f * Z;
		z =  0.000573063005776f * X - 0.002101231021178f * Y + 0.010888197109740f * Z;*/
#else
		x = s.value(0);
		y = s.value(1);
		z = s.value(2);
#endif
	}

	float RGBConverter::luminance(float r, float g, float b)
	{
		return 0.2126f*r + 0.7152f*g + 0.0722f*b;
	}

	void RGBConverter::gamma(float &x, float &y, float &z)
	{
		x = (x <= 0.0031308f) ? 12.92f*x : (1.055f*pow(x, 0.4166666f) - 0.055f);
		y = (y <= 0.0031308f) ? 12.92f*y : (1.055f*pow(y, 0.4166666f) - 0.055f);
		z = (z <= 0.0031308f) ? 12.92f*z : (1.055f*pow(z, 0.4166666f) - 0.055f);
	}

	constexpr int RGB_TO_SPEC_SAMPLING_COUNT = 32;
	float RGB_TO_SPEC[RGB_TO_SPEC_SAMPLING_COUNT * 7] = {
	/*  WHITE        CYAN           MAGENTA      YELLOW       RED           GREEN            BLUE*/
		1.05719,     1.01427,       1.00752,     0.000102101, 0.175693,     9.10708e-06,     0.998984,     
		1.05719,     1.00542,       1.00809,     0.000177382, 0.195608,     0.000511201,     0.996503,     
		1.05719,     0.991603,      1.00811,     0.00036854,  0.204929,     0.000307951,     0.99651,     
		1.05719,     0.985264,      1.00812,     0.000130937, 0.204491,     4.60046e-06,     0.999295,     
		1.05715,     0.984397,      1.00812,     5.22013e-14, 0.17315,     -5.1867e-07,      1,     
		1.05707,     1.0046,        1.00812,     1.09889e-05, 0.0101469,   -5.87786e-07,     1,     
		1.05719,     1.04619,       1.00812,     0.0158029,   0.000306284,  1.72895e-07,     0.999998,     
		1.05719,     1.05195,       1.00812,     0.114735,   -2.49458e-07,  2.48788e-07,     1,     
		1.05719,     1.05194,       1.00807,     0.2984,      1.81244e-14,  0.202783,        0.851987,     
		1.05719,     1.05195,       0.631566,    0.538379,    5.86616e-07,  0.679807,        0.578973,     
		1.05719,     1.05193,       0.144311,    0.788501,   -2.92525e-07,  1,               0.289561,     
		1.05719,     1.05195,       3.08048e-07, 0.990266,   -3.27536e-07,  1,               0.0686266,     
		1.05719,     1.05195,       1.95948e-06, 1.04637,    -2.958e-07,    1,               1.54833e-06,     
		1.05719,     1.05195,       3.3347e-07,  1.04637,     3.30912e-06,  1,               1.84044e-13,     
		1.05719,     1.05194,      -9.58122e-14, 1.04636,    -1.83315e-07,  0.999999,        3.37366e-06,     
		1.05719,     1.05193,       0.0500931,   1.04637,    -2.41982e-07,  0.999999,        2.85665e-06,     
		1.05719,     0.617618,      0.557766,    1.04637,     0.463971,     0.616892,        5.88666e-06,     
		1.05704,     0.126669,      1.00734,     1.04637,     1.01463,      0.160879,        0.000119138,     
		1.05696,    -4.86604e-13,   1.00812,     1.04637,     1.01463,     -6.58153e-07,     0.014505,     
		1.0569,      1.82139e-06,   1.00811,     1.04637,     1.01463,      1.69803e-08,     0.0349575,     
		1.05683,     2.27072e-05,   1.00808,     1.04637,     1.01463,      4.69589e-07,     0.0491743,     
		1.05681,     0.000193157,   1.0081,      1.04637,     1.01463,     -3.9055e-07,      0.0557459,     
		1.05681,     9.78903e-05,   1.00255,     1.04637,     1.01463,      2.23979e-05,     0.057073,     
		1.05682,     5.96214e-05,   0.873871,    1.04637,     1.01456,     -8.28394e-07,     0.0561998,     
		1.05684,     5.69876e-06,   0.728075,    1.04636,     1.01461,      0.00110963,      0.0545057,     
		1.05686,     0.000126662,   0.586804,    1.04617,     1.01455,      4.13701e-05,     0.0521155,     
		1.05689,     0.00125526,    0.439099,    1.04604,     1.01331,      0.000248335,     0.049933,     
		1.05691,     0.00117356,    0.351158,    1.04492,     1.00706,      0.00153085,      0.0475245,     
		1.05693,     0.0120824,     0.262838,    1.04141,     1.00584,      0.000284403,     0.0451013,     
		1.05696,     0.0233264,     0.176065,    1.03828,     1.00571,      6.73492e-06,     0.048456,     
		1.057,       0.0345575,     0.0883558,   1.03626,     1.00857,      0.00032172,      0.0504819,
		1.05701,     0.0459001,     0.000326262, 1.03518,     1.01326,      0.000405666,     0.0512405,
	};

	Spectrum RGBConverter::White;
	Spectrum RGBConverter::Cyan;
	Spectrum RGBConverter::Magenta;
	Spectrum RGBConverter::Yellow;
	Spectrum RGBConverter::Red;
	Spectrum RGBConverter::Green;
	Spectrum RGBConverter::Blue;

	void RGBConverter::init()
	{
#ifndef PR_NO_SPECTRAL
		constexpr float Q = RGB_TO_SPEC_SAMPLING_COUNT / (float)Spectrum::SAMPLING_COUNT;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float findex = i*Q;
			int index = (int)std::floor(findex);

			if (index < RGB_TO_SPEC_SAMPLING_COUNT - 1)
			{
				float t = findex - index;
				float nt = 1 - t;

				White.setValue(i, RGB_TO_SPEC[index * 7] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7] * t);
				Cyan.setValue(i, RGB_TO_SPEC[index * 7 + 1] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 1] * t);
				Magenta.setValue(i, RGB_TO_SPEC[index * 7 + 2] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 2] * t);
				Yellow.setValue(i, RGB_TO_SPEC[index * 7 + 3] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 3] * t);
				Red.setValue(i, RGB_TO_SPEC[index * 7 + 4] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 4] * t);
				Green.setValue(i, RGB_TO_SPEC[index * 7 + 5] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 5] * t);
				Blue.setValue(i, RGB_TO_SPEC[index * 7 + 6] * nt 
					+ RGB_TO_SPEC[(index + 1) * 7 + 6] * t);
			}
			else
			{
				White.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT-1) * 7]);
				Cyan.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 1]);
				Magenta.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 2]);
				Yellow.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 3]);
				Red.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 4]);
				Green.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 5]);
				Blue.setValue(i, RGB_TO_SPEC[(RGB_TO_SPEC_SAMPLING_COUNT - 1) * 7 + 6]);
			}
		}
#else
		White.setValue(0, 1); White.setValue(1, 1); White.setValue(2, 1);
		Cyan.setValue(0, 0); Cyan.setValue(1, 1); Cyan.setValue(2, 1);
		Magenta.setValue(0, 1); Magenta.setValue(1, 0); Magenta.setValue(2, 1);
		Yellow.setValue(0, 1); Yellow.setValue(1, 1); Yellow.setValue(1, 0);
		Red.setValue(0, 1);
		Green.setValue(1, 1);
		Blue.setValue(2, 1);
#endif
	}

	Spectrum RGBConverter::toSpec(float r, float g, float b)
	{
		Spectrum spec;

#ifndef PR_NO_SPECTRAL
		if (r <= g && r <= b)
		{
			spec = r * White;

			if (g <= b)
			{
				spec += (g - r) * Cyan;
				spec += (b - g) * Blue;
			}
			else
			{
				spec += (b - r) * Cyan;
				spec += (g - b) * Green;
			}
		}
		else if (g <= r && g <= b)
		{
			spec = g * White;
			if (r <= b)
			{
				spec += (r - g) * Magenta;
				spec += (b - r) * Blue;
			}
			else
			{
				spec += (b - g) * Magenta;
				spec += (r - b) * Red;
			}
		}
		else
		{
			spec = b * White;
			if (r <= g)
			{
				spec += (r - b) * Yellow;
				spec += (g - r) * Green;
			}
			else
			{
				spec += (g - b) * Yellow;
				spec += (r - g) * Red;
			}
		}

		//spec *= 0.94f;
		//spec *= 0.8f;
		//return spec.clamp();
#else
		spec.setValue(0, r);
		spec.setValue(1, g);
		spec.setValue(2, b);
#endif
		return spec;
	}
}