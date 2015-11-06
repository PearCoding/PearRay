#pragma once

#include "ColorSpaceConverter.h"

namespace PR
{
	class PR_LIB IntensityConverter : public ColorSpaceConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z);
	};
}