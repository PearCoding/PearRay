#pragma once

#include "Config.h"

namespace PR
{
	class Spectrum;
	class ColorSpaceConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z) = 0;
	};
}