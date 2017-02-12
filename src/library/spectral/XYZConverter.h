#pragma once

#include "Spectrum.h"

namespace PR
{
	class GPU;
	class Spectrum;
	class PR_LIB XYZConverter
	{
		PR_CLASS_NON_COPYABLE(XYZConverter);

	public:
		//len(src) == SAMPLING_COUNT
		static void convert(const float* src, float &x, float &y, float &z);
		static inline void convert(const Spectrum& s, float &x, float &y, float &z)
		{
			convert(s.ptr(), x, y, z);
		}

		//len(src) == SAMPLING_COUNT
		static void convertXYZ(const float* src, float &X, float &Y, float &Z);
		static inline void convertXYZ(const Spectrum& s, float &X, float &Y, float &Z)
		{
			convertXYZ(s.ptr(), X, Y, Z);
		}
	};
}
