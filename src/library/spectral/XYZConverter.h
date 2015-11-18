#pragma once

#include "Config.h"

namespace PR
{
	class Spectrum;
	class PR_LIB XYZConverter
	{
		PR_CLASS_NON_COPYABLE(XYZConverter);

	public:
		/* E Illuminant */
		static void convert(const Spectrum& s, float &x, float &y, float &z);
		static void convertXYZ(const Spectrum& s, float &X, float &Y, float &Z);

		static void init();

	private:
		static float N;// Illuminent Factor
	};
}