#include "RGBConverter.h"
#include "XYZConverter.h"

#include "SpectrumDescriptor.h"

#include "Diagnosis.h"

namespace PR {
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
	return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

void RGBConverter::gamma(float& x, float& y, float& z)
{
	x = (x <= 0.0031308f) ? 12.92f * x : (1.055f * pow(x, 0.4166666f) - 0.055f);
	y = (y <= 0.0031308f) ? 12.92f * y : (1.055f * pow(y, 0.4166666f) - 0.055f);
	z = (z <= 0.0031308f) ? 12.92f * z : (1.055f * pow(z, 0.4166666f) - 0.055f);
}

void RGBConverter::toSpec(Spectrum& spec, float r, float g, float b)
{
	float x, y, z;
	toXYZ(r, g, b, x, y, z);

	XYZConverter::toSpec(spec, x, y, z);
}
}
