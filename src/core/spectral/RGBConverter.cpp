#include "RGBConverter.h"

namespace PR {
void RGBConverter::toXYZ(float r, float g, float b, float& x, float& y, float& z)
{
	x = 4.123908e-01f * r + 3.575843e-01f * g + 1.804808e-01f * b;
	y = 2.126390e-01f * r + 7.151687e-01f * g + 7.219232e-02f * b;
	z = 1.933082e-02f * r + 1.191948e-01f * g + 9.505322e-01f * b;

	x = std::max(0.0f, x);
	y = std::max(0.0f, y);
	z = std::max(0.0f, z);
}

void RGBConverter::fromXYZ(float x, float y, float z, float& r, float& g, float& b)
{
	r = 3.240970e+00f * x - 1.537383e+00f * y - 4.986108e-01f * z;
	g = -9.692436e-01f * x + 1.875968e+00f * y + 4.155506e-02f * z;
	b = 5.563008e-02f * x - 2.039770e-01f * y + 1.056972e+00f * z;

	r = std::max(0.0f, r);
	g = std::max(0.0f, g);
	b = std::max(0.0f, b);
}

void RGBConverter::fromXYZ(const float* PR_RESTRICT xyzIn, float* PR_RESTRICT rgbOut, size_t outElems, size_t pixelcount)
{
	PR_ASSERT(outElems >= 3, "Expected atleast an RGB buffer");
	PR_ASSERT(xyzIn != rgbOut, "Inplace transform from xyz to rgb not supported");
	PR_OPT_LOOP
	for (size_t i = 0; i < pixelcount; ++i)
		fromXYZ(xyzIn[i * 3 + 0], xyzIn[i * 3 + 1], xyzIn[i * 3 + 2], rgbOut[i * outElems + 0], rgbOut[i * outElems + 1], rgbOut[i * outElems + 2]);
}

float RGBConverter::luminance(float r, float g, float b)
{
	return PR_LUMINOSITY_RED * r + PR_LUMINOSITY_GREEN * g + PR_LUMINOSITY_BLUE * b;
}

constexpr float GAM_T  = 0.0031308f;
constexpr float LIN_T  = 0.04045f;
constexpr float POW_F  = 2.4f;
constexpr float LIN_O  = 0.055f;
constexpr float LIN_F1 = 12.92f;
constexpr float LIN_F2 = 1.055f;
void RGBConverter::gamma(float& x, float& y, float& z)
{
	x = (x <= GAM_T) ? LIN_F1 * x : (LIN_F2 * std::pow(x, 1 / POW_F) - LIN_O);
	y = (y <= GAM_T) ? LIN_F1 * y : (LIN_F2 * std::pow(y, 1 / POW_F) - LIN_O);
	z = (z <= GAM_T) ? LIN_F1 * z : (LIN_F2 * std::pow(z, 1 / POW_F) - LIN_O);
}

void RGBConverter::linearize(float& x, float& y, float& z)
{
	x = (x <= LIN_T) ? x / LIN_F1 * x : std::pow((x + LIN_O) / LIN_F2, POW_F);
	y = (y <= LIN_T) ? y / LIN_F1 * y : std::pow((y + LIN_O) / LIN_F2, POW_F);
	z = (z <= LIN_T) ? z / LIN_F1 * z : std::pow((z + LIN_O) / LIN_F2, POW_F);
}
} // namespace PR
