#pragma once

#include "Config.h"

namespace PR
{
	class Spectrum;
	class PR_LIB_INLINE ColorSpaceConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z) = 0;
	};
}