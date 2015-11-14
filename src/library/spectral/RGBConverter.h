#pragma once

#include "XYZConverter.h"

namespace PR
{
	class PR_LIB RGBConverter : public XYZConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z) override;
		virtual Spectrum toSpec(float x, float y, float z);
	};
}