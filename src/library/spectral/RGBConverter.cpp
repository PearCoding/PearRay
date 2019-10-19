#include "RGBConverter.h"
#include "XYZConverter.h"

#include "SpectrumDescriptor.h"

namespace PR {
// D65 sRGB
void RGBConverter::convert(uint32 samples, const float* src, float& x, float& y, float& z)
{
	float X, Y, Z;
	XYZConverter::convertXYZ(samples, src, X, Y, Z);
	fromXYZ(X, Y, Z, x, y, z);
}

void RGBConverter::toXYZ(float r, float g, float b, float& x, float& y, float& z)
{
	x = 4.123908e-01 * r + 3.575843e-01 * g + 1.804808e-01 * b;
	y = 2.126390e-01 * r + 7.151687e-01 * g + 7.219232e-02 * b;
	z = 1.933082e-02 * r + 1.191948e-01 * g + 9.505322e-01 * b;

	x = std::max(0.0f, x);
	y = std::max(0.0f, y);
	z = std::max(0.0f, z);
}

void RGBConverter::fromXYZ(float x, float y, float z, float& r, float& g, float& b)
{
	r = 3.240970e+00 * x - 1.537383e+00 * y - 4.986108e-01 * z;
	g = -9.692436e-01 * x + 1.875968e+00 * y + 4.155506e-02 * z;
	b = 5.563008e-02 * x - 2.039770e-01 * y + 1.056972e+00 * z;

	r = std::max(0.0f, r);
	g = std::max(0.0f, g);
	b = std::max(0.0f, b);
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
	x = (x <= GAM_T) ? LIN_F1 * x : (LIN_F2 * pow(x, 1 / POW_F) - LIN_O);
	y = (y <= GAM_T) ? LIN_F1 * y : (LIN_F2 * pow(y, 1 / POW_F) - LIN_O);
	z = (z <= GAM_T) ? LIN_F1 * z : (LIN_F2 * pow(z, 1 / POW_F) - LIN_O);
}

void RGBConverter::linearize(float& x, float& y, float& z)
{
	x = (x <= LIN_T) ? x / LIN_F1 * x : pow((x + LIN_O) / LIN_F2, POW_F);
	y = (y <= LIN_T) ? y / LIN_F1 * y : pow((y + LIN_O) / LIN_F2, POW_F);
	z = (z <= LIN_T) ? z / LIN_F1 * z : pow((z + LIN_O) / LIN_F2, POW_F);
}

void RGBConverter::toSpec(Spectrum& spec, float r, float g, float b)
{
	float x, y, z;
	toXYZ(r, g, b, x, y, z);

	XYZConverter::toSpec(spec, x, y, z);
}

float RGBConverter::toSpecIndex(uint32 samples, uint32 index, float r, float g, float b)
{
	float x, y, z;
	toXYZ(r, g, b, x, y, z);

	return XYZConverter::toSpecIndex(samples, index, x, y, z);
}
} // namespace PR
