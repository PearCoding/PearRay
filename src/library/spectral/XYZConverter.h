#pragma once

#include "Config.h"

namespace PR
{
	class GPU;
	class Spectrum;
	class PR_LIB XYZConverter
	{
		PR_CLASS_NON_COPYABLE(XYZConverter);

	public:
		static void convert(const Spectrum& s, float &x, float &y, float &z);
		static void convertXYZ(const Spectrum& s, float &X, float &Y, float &Z);
	};
}