#pragma once

#include "Spectrum.h"

#include "gpu/GPU.h"

namespace PR {
class GPU;
class PR_LIB RGBConverter {
	PR_CLASS_NON_COPYABLE(RGBConverter);

public:
	/* D65 sRGB (linear) */
	// len(src) = SAMPLING_COUNT
	static void convert(uint32 samples, const float* src, float& x, float& y, float& z);
	static inline void convert(const Spectrum& s, float& x, float& y, float& z)
	{
		convert(s.samples(), s.c_ptr(), x, y, z);
	}

	static void toXYZ(float r, float g, float b, float& x, float& y, float& z);
	static void fromXYZ(float x, float y, float z, float& r, float& g, float& b);

	static float luminance(float r, float g, float b);
	static void gamma(float& x, float& y, float& z);

	static void toSpec(Spectrum& spec, float x, float y, float z);
};
}
