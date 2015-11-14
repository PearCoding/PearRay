#pragma once

#include "Config.h"

namespace PR
{
	class Spectrum;
	class PR_LIB IntensityConverter
	{
	public:
		void convert(const Spectrum& s, float &x, float &y, float &z);
	};
}