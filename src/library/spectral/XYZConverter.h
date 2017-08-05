#pragma once

#include "Spectrum.h"

namespace PR {
class GPU;
class Spectrum;
class PR_LIB XYZConverter {
	PR_CLASS_NON_COPYABLE(XYZConverter);

public:
	//len(src) == SAMPLING_COUNT
	static void convert(const float* src, float& x, float& y);
	static inline void convert(const Spectrum& s, float& x, float& y)
	{
		convert(s.c_ptr(), x, y);
	}

	static float luminance(float x, float y, float z) { return y; }

	//len(src) == SAMPLING_COUNT
	static void convertXYZ(const float* src, float& X, float& Y, float& Z);
	static inline void convertXYZ(const Spectrum& s, float& X, float& Y, float& Z)
	{
		convertXYZ(s.c_ptr(), X, Y, Z);
	}

	static void toNorm(float X, float Y, float Z, float& x, float& y)
	{
		const float B = X + Y + Z;
		x			  = X / B;
		y			  = Y / B;
	}

	static Spectrum toSpec(float x, float y, float z);
};
}
