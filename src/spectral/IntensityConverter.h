#pragma once

#include "ColorSpaceConverter.h"

namespace PR
{
	class IntensityConverter : public ColorSpaceConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z);
	};
}