#pragma once

#include "Config.h"

namespace PR
{
	class Spectrum;
	class PR_LIB XYZConverter
	{
	public:
		virtual void convert(const Spectrum& s, float &x, float &y, float &z);

	protected:
		void convertXYZ(const Spectrum& s, float &X, float &Y, float &Z);
	};
}