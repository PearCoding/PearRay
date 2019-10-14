#pragma once

#include "Spectrum.h"

namespace PR {

// SRGB Values
constexpr float PR_LUMINOSITY_RED   = 0.2126f;
constexpr float PR_LUMINOSITY_GREEN = 0.7152f;
constexpr float PR_LUMINOSITY_BLUE  = 0.0722f;

class PR_LIB RGBConverter {
	PR_CLASS_NON_COPYABLE(RGBConverter);

public:
	/* D65 sRGB (linear) */
	static void convert(uint32 samples,
						const float* src, float& x, float& y, float& z);
	static inline void convert(const Spectrum& s, float& x, float& y, float& z)
	{
		convert(s.samples(), s.c_ptr(), x, y, z);
	}

	static void toXYZ(float r, float g, float b, float& x, float& y, float& z);
	static void fromXYZ(float x, float y, float z, float& r, float& g, float& b);

	static float luminance(float r, float g, float b);
	static void gamma(float& x, float& y, float& z);

	static void toSpec(Spectrum& spec, float x, float y, float z);
	static float toSpecIndex(uint32 samples, uint32 index, float x, float y, float z);
};
} // namespace PR
