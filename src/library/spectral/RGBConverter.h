#pragma once

#include "ColorSpaceConverter.h"

namespace PR
{
	class PR_LIB RGBConverter : public ColorSpaceConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z);
	};
}