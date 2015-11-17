#pragma once

#include "Spectrum.h"

namespace PR
{
	class PR_LIB RGBConverter
	{
		PR_CLASS_NON_COPYABLE(RGBConverter);

	public:
		static void convert(const Spectrum& s, float &x, float &y, float &z);
		static Spectrum toSpec(float x, float y, float z);
		static void init();

		static Spectrum White;
		static Spectrum Cyan;
		static Spectrum Magenta;
		static Spectrum Yellow;
		static Spectrum Red;
		static Spectrum Green;
		static Spectrum Blue;
	};
}