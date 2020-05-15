#pragma once

#include "PR_Config.h"

namespace PR {

// SRGB Values
constexpr float PR_LUMINOSITY_RED   = 0.2126f;
constexpr float PR_LUMINOSITY_GREEN = 0.7152f;
constexpr float PR_LUMINOSITY_BLUE  = 0.0722f;

// D65 sRGB
class PR_LIB_CORE RGBConverter {
	PR_CLASS_NON_COPYABLE(RGBConverter);

public:
	static void toXYZ(float r, float g, float b, float& x, float& y, float& z);
	static void fromXYZ(float x, float y, float z, float& r, float& g, float& b);

	/// Input buffer has to be of size pixelcount*3, output buffer of pixelcount*outElems
	static void fromXYZ(const float* xyzIn, float* rgbOut, size_t outElems, size_t pixelcount);

	static float luminance(float r, float g, float b);
	static void gamma(float& x, float& y, float& z);
	static void linearize(float& x, float& y, float& z);
};
} // namespace PR
