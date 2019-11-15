#include "spectral/RGBConverter.h"
#include "spectral/SpectrumDescriptor.h"
#include "spectral/XYZConverter.h"

#include "Test.h"

using namespace PR;

namespace PR {
extern void barycentricTriangle(double px, double py,
								double x1, double y1, double x2, double y2, double x3, double y3, double invArea,
								double& s, double& t);
}

constexpr double SPEC_EPS = 0.00001;

PR_BEGIN_TESTCASE(Spectrum)
PR_TEST("Set/Get")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());

	spec.setValue(52, 1);
	PR_CHECK_EQ(spec.value(52), 1);
}
PR_TEST("Fill")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	spec.fill(1);

	for (uint32 i = 0; i < spec.samples(); ++i)
		PR_CHECK_EQ(spec(i), 1);
}
PR_TEST("Max/Min/Avg/Sum")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral(), 0);

	spec(52) = 1;
	spec(42) = -1;

	PR_CHECK_EQ(spec.max(), 1);
	PR_CHECK_EQ(spec.min(), 0);// Amplitude!
	PR_CHECK_EQ(spec.avg(), 0);
	PR_CHECK_EQ(spec.sum(), 0);
	PR_CHECK_EQ(spec.sqrSum(), 2);
}
PR_TEST("BarycentricTriangle")
{
	const auto areaF = [](double x1, double y1, double x2, double y2, double x3, double y3) {
		return 0.5 * (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2);
	};

	double area = areaF(0, 0, 1, 0, 0, 1);
	double s, t;
	barycentricTriangle(0.5, 0.5, 0, 0, 1, 0, 0, 1, 1 / (2 * area), s, t);

	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0.5);
	PR_CHECK_NEARLY_EQ(area, 0.5);

	barycentricTriangle(0, 0.5, 0, 0, 1, 0, 0, 1, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, 0);
	PR_CHECK_NEARLY_EQ(t, 0.5);

	area = areaF(0, 0, 2, 0, 0, 2);
	barycentricTriangle(1, 0, 0, 0, 2, 0, 0, 2, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0);

	area = areaF(0, 0, 2, 0, 0, 2);
	barycentricTriangle(-1, 0, 0, 0, 2, 0, 0, 2, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, -0.5);
	PR_CHECK_NEARLY_EQ(t, 0);
}
PR_TEST("BarycentricTriangle Bad Order")
{
	const auto areaF = [](double x1, double y1, double x2, double y2, double x3, double y3) {
		return 0.5 * (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2);
	};

	double area = areaF(1, 0, 0, 0, 0, 1);
	double s, t;
	barycentricTriangle(0.5, 0.5, 1, 0, 0, 0, 0, 1, 1 / (2 * area), s, t);

	PR_CHECK_NEARLY_EQ(s, 0);
	PR_CHECK_NEARLY_EQ(t, 0.5);
	PR_CHECK_NEARLY_EQ(area, -0.5);

	barycentricTriangle(0, 0.5, 1, 0, 0, 0, 0, 1, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0.5);

	area = areaF(2, 0, 0, 0, 0, 2);
	barycentricTriangle(1, 0, 2, 0, 0, 0, 0, 2, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0);

	area = areaF(2, 0, 0, 0, 0, 2);
	barycentricTriangle(-1, 0, 2, 0, 0, 0, 0, 2, 1 / (2 * area), s, t);
	PR_CHECK_NEARLY_EQ(s, 1.5);
	PR_CHECK_NEARLY_EQ(t, 0);
}
PR_TEST("XYZ")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	spec.fill(1);

	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ_EPS(X, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Z, 1, SPEC_EPS);
}
PR_TEST("-> sRGB")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	spec.fill(1);

	float X, Y, Z;
	PR::RGBConverter::convert(spec, X, Y, Z);

	PR_CHECK_NEARLY_EQ_EPS(X, 1.204976106621948, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, 0.948278897530075, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Z, 0.908624594035630, SPEC_EPS);
}
PR_TEST("<-> sRGB [White]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 1, 1, 1);

	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 1, SPEC_EPS);

	const float q = (R - 1) * (R - 1) + (G - 1) * (G - 1) + (B - 1) * (B - 1);
	std::cout << "Error [White]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Cyan]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 0, 1, 1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 1, SPEC_EPS);

	const float q = (R - 0) * (R - 0) + (G - 1) * (G - 1) + (B - 1) * (B - 1);
	std::cout << "Error [Cyan]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Magenta]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 1, 0, 1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 1, SPEC_EPS);

	const float q = (R - 1) * (R - 1) + (G - 0) * (G - 0) + (B - 1) * (B - 1);
	std::cout << "Error [Magenta]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Yellow]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 1, 1, 0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 0, SPEC_EPS);

	const float q = (R - 1) * (R - 1) + (G - 1) * (G - 1) + (B - 0) * (B - 0);
	std::cout << "Error [Yellow]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Red]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 1, 0, 0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 0, SPEC_EPS);

	const float q = (R - 1) * (R - 1) + (G - 0) * (G - 0) + (B - 0) * (B - 0);
	std::cout << "Error [Red]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Green]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 0, 1, 0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 1, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 0, SPEC_EPS);

	const float q = (R - 0) * (R - 0) + (G - 1) * (G - 1) + (B - 0) * (B - 0);
	std::cout << "Error [Green]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Blue]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 0, 0, 1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 0, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 1, SPEC_EPS);

	const float q = (R - 0) * (R - 0) + (G - 0) * (G - 0) + (B - 1) * (B - 1);
	std::cout << "Error [Blue]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Custom]")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 0.8f, 0.5f, 0.2f);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 0.8f, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 0.5f, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 0.2f, SPEC_EPS);

	const float q = (R - 0.8f) * (R - 0.8f) + (G - 0.5f) * (G - 0.5f) + (B - 0.2f) * (B - 0.2f);
	std::cout << "Error [Custom]: " << q << std::endl;
}

PR_TEST("-> XYZ")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	spec.fill(1);

	float X, Y;
	PR::XYZConverter::convert(spec, X, Y);
	PR_CHECK_NEARLY_EQ_EPS(X, 1 / 3.0f, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, 1 / 3.0f, SPEC_EPS);
}

PR_TEST("<-> XYZ")
{
	float x = 1 / 3.0f;
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::XYZConverter::toSpec(spec, x, x, x);
	float X, Y;
	PR::XYZConverter::convert(spec, X, Y);
	PR_CHECK_NEARLY_EQ_EPS(X, x, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, x, SPEC_EPS);
}
PR_TEST("<-> XYZ")
{
	float x = 0.4f;
	float y = 0.2f;
	float z = 0.4f;
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::XYZConverter::toSpec(spec, x, y, z);
	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ_EPS(X, x, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, y, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Z, z, SPEC_EPS);
}
PR_TEST("<-> XYZ")
{
	float x = 0.2f;
	float y = 0.2f;
	float z = 0.4f;
	float b = x + y + z;
	x /= b;
	y /= b;
	z /= b;
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::XYZConverter::toSpec(spec, x, y, z);
	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ_EPS(X, x, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Y, y, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(Z, z, SPEC_EPS);
}
PR_TEST("XYZ->RGB->XYZ")
{
	float X = 1, Y = 1, Z = 1;
	float r, g, b;
	float x, y, z;
	PR::RGBConverter::fromXYZ(X, Y, Z, r, g, b);
	PR::RGBConverter::toXYZ(r, g, b, x, y, z);
	PR_CHECK_NEARLY_EQ_EPS(x, X, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(y, Y, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(z, Z, SPEC_EPS);
}
PR_TEST("RGB->XYZ->RGB")
{
	float R = 1, G = 1, B = 1;
	float r, g, b;
	float x, y, z;
	PR::RGBConverter::toXYZ(R, B, B, x, y, z);
	PR::RGBConverter::fromXYZ(x, y, z, r, b, g);
	PR_CHECK_NEARLY_EQ_EPS(r, R, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(g, B, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(b, G, SPEC_EPS);
}
PR_TEST("Gray")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral(), 1);
	spec *= 0.5f;

	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ_EPS(R, 0.5, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(G, 0.5, SPEC_EPS);
	PR_CHECK_NEARLY_EQ_EPS(B, 0.5, SPEC_EPS);
}
PR_TEST("White Norm")
{
	Spectrum white = PR::Spectrum::white(SpectrumDescriptor::createStandardSpectral());
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	PR::RGBConverter::toSpec(spec, 1, 1, 1);

	Spectrum Diff = white - spec;
	float Err	 = Diff.sqrSum();
	std::cout << " Reflectance White Error " << Err << " and max Error " << Diff.max() << " and avg Error " << Diff.avg() << std::endl;
	PR_CHECK_LESS(Err, 0.2);
}

PR_END_TESTCASE()

PRT_BEGIN_MAIN
PRT_TESTCASE(Spectrum);
PRT_END_MAIN