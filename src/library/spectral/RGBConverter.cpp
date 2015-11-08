#include "RGBConverter.h"
#include "Spectrum.h"

#include "PearMath.h"

#include <iostream>

namespace PR
{
	/* Source: https://en.wikipedia.org/wiki/File:CIE1931_RGBCMF.svg */
	float NM_TO_RGB[Spectrum::SAMPLING_COUNT*3] = 
	{/* Red		Green	Blue */
		0,		0,		0,		// 380 nm
		0,		0,		0,		// 390 nm
		0,		0,		0.01f,	// 400 nm [BLUE START]
		0,		0,		0.04f,	// 410 nm 
		0,		0,		0.105f,	// 420 nm
		0,		0,		0.230f,	// 430 nm
		-0.005f,0,		0.310f,	// 440 nm [RED START]
		-0.015f,0.01f,	0.310f,	// 450 nm [GREEN START]
		-0.03f,	0.015f,	0.285f,	// 460 nm
		-0.04f,	0.02f,	0.225f,	// 470 nm
		-0.05f,	0.04f,	0.145f,	// 480 nm
		-0.055f,0.055f,	0.085f,	// 490 nm
		-0.07f,	0.08f,	0.050f,	// 500 nm
		-0.1f,	0.13f,	0.030f,	// 510 nm
		-0.1f,	0.175f,	0.015f,	// 520 nm
		-0.08f,	0.205f,	0.005f,	// 530 nm [BLUE END]
		-0.035f,0.210f,	0,		// 540 nm
		0.02f,	0.205f,	0,		// 550 nm
		0.07f,	0.195f,	0,		// 560 nm
		0.16f,	0.165f,	0,		// 570 nm
		0.24f,	0.140f,	0,		// 580 nm
		0.3f,	0.105f,	0,		// 590 nm
		0.33f,	0.065f,	0,		// 600 nm
		0.34f,	0.04f,	0,		// 610 nm
		0.32f,	0.02f,	0,		// 620 nm
		0.23f,	0.01f,	0,		// 630 nm
		0.155f,	0.005f,	0,		// 640 nm [GREEN END]
		0.11f,	0,		0,		// 650 nm
		0.06f,	0,		0,		// 660 nm
		0.03f,	0,		0,		// 670 nm
		0.015f,	0,		0,		// 680 nm
		0.01f,	0,		0,		// 690 nm
		0.005f,	0,		0,		// 700 nm [RED END]
		0,		0,		0,		// 710 nm
		0,		0,		0,		// 720 nm
		0,		0,		0,		// 730 nm
		0,		0,		0,		// 740 nm
		0,		0,		0,		// 750 nm
		0,		0,		0,		// 760 nm
		0,		0,		0,		// 770 nm
		0,		0,		0,		// 780 nm
	};

	const int RGB_SAMPLE_RATE = 2;

	void RGBConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		x = 0;
		y = 0;
		z = 0;

		for (uint32 w = 400 - Spectrum::WAVELENGTH_START - Spectrum::WAVELENGTH_STEP;
			w <= 700 - Spectrum::WAVELENGTH_START + Spectrum::WAVELENGTH_STEP;
				w += RGB_SAMPLE_RATE)// Works without checks because 700 < Spectrum::WAVELENGTH_END
		{
			size_t i = w / Spectrum::WAVELENGTH_STEP;
			float t = (w % Spectrum::WAVELENGTH_STEP) / (float) Spectrum::WAVELENGTH_STEP;

			float val = s.value(i)*(1 - t) + s.value(i + 1)*t;

			x += val * (NM_TO_RGB[3 * i] * (1 - t) + NM_TO_RGB[3 * (i + 1)] * t);
			y += val * (NM_TO_RGB[3 * i + 1] * (1 - t) + NM_TO_RGB[3 * (i + 1) + 1] * t);
			z += val * (NM_TO_RGB[3 * i + 2] * (1 - t) + NM_TO_RGB[3 * (i + 1) + 2] * t);
		}

		//std::cout << x << " " << y << " " << z << std::endl;
		x = PM::pm_MaxT<float>(0, x) * RGB_SAMPLE_RATE;
		y = PM::pm_MaxT<float>(0, y) * RGB_SAMPLE_RATE;
		z = PM::pm_MaxT<float>(0, z) * RGB_SAMPLE_RATE;
	}
}