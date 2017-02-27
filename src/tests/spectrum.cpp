#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include "Test.h"

using namespace PR;

namespace PR {
extern void barycentricTriangle(double px, double py,
		double x1, double y1, double x2, double y2, double x3, double y3, double invArea,
		double& s, double& t);
}

PR_BEGIN_TESTCASE(Spectrum)
PR_TEST("Set/Get")
{
	PR::Spectrum spec;

	spec.setValueAtWavelength(640, 1);
	PR_CHECK_EQ(spec.value(52), 1);
}
PR_TEST("Fill")
{
	PR::Spectrum spec;
	spec.fill(1);

	for(uint32 i = 0; i < PR::Spectrum::SAMPLING_COUNT; ++i)
		PR_CHECK_EQ(spec.value(i), 1);
}
PR_TEST("BarycentricTriangle")
{
	const auto areaF = [] (double x1, double y1, double x2, double y2, double x3, double y3) {
		return 0.5*(x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2);
	};

	double area = areaF(0,0,1,0,0,1);
	double s, t;
	barycentricTriangle(0.5,0.5, 0,0, 1,0, 0,1, 1/(2*area), s,t);

	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0.5);
	PR_CHECK_NEARLY_EQ(area, 0.5);

	barycentricTriangle(0,0.5, 0,0, 1,0, 0,1, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, 0);
	PR_CHECK_NEARLY_EQ(t, 0.5);

	area = areaF(0,0,2,0,0,2);
	barycentricTriangle(1,0, 0,0, 2,0, 0,2, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0);

	area = areaF(0,0,2,0,0,2);
	barycentricTriangle(-1,0, 0,0, 2,0, 0,2, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, -0.5);
	PR_CHECK_NEARLY_EQ(t, 0);
}
PR_TEST("BarycentricTriangle Bad Order")
{
	const auto areaF = [] (double x1, double y1, double x2, double y2, double x3, double y3) {
		return 0.5*(x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2);
	};

	double area = areaF(1,0,0,0,0,1);
	double s, t;
	barycentricTriangle(0.5,0.5, 1,0, 0,0, 0,1, 1/(2*area), s,t);

	PR_CHECK_NEARLY_EQ(s, 0);
	PR_CHECK_NEARLY_EQ(t, 0.5);
	PR_CHECK_NEARLY_EQ(area, -0.5);

	barycentricTriangle(0,0.5, 1,0, 0,0, 0,1, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0.5);

	area = areaF(2,0,0,0,0,2);
	barycentricTriangle(1,0, 2,0, 0,0, 0,2, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, 0.5);
	PR_CHECK_NEARLY_EQ(t, 0);

	area = areaF(2,0,0,0,0,2);
	barycentricTriangle(-1,0, 2,0, 0,0, 0,2, 1/(2*area), s,t);
	PR_CHECK_NEARLY_EQ(s, 1.5);
	PR_CHECK_NEARLY_EQ(t, 0);
}
PR_TEST("XYZ")
{
	PR::Spectrum spec;
	spec.fill(1);

	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ(X, 1);
	PR_CHECK_NEARLY_EQ(Y, 1);
	PR_CHECK_NEARLY_EQ(Z, 1);
}
PR_TEST("-> sRGB")
{
	PR::Spectrum spec;
	spec.fill(1);

	float X, Y, Z;
	PR::RGBConverter::convert(spec, X, Y, Z);

	PR_CHECK_NEARLY_EQ(X, 1.204976106621948);
	PR_CHECK_NEARLY_EQ(Y, 0.948278897530075);
	PR_CHECK_NEARLY_EQ(Z, 0.908624594035630);
}
PR_TEST("<-> sRGB [White]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(1,1,1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 1);
	PR_CHECK_NEARLY_EQ(G, 1);
	PR_CHECK_NEARLY_EQ(B, 1);

	const float q = (R-1)*(R-1) + (G-1)*(G-1) + (B-1)*(B-1);
	std::cout << "Error [White]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Cyan]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(0,1,1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 0);
	PR_CHECK_NEARLY_EQ(G, 1);
	PR_CHECK_NEARLY_EQ(B, 1);

	const float q = (R-0)*(R-0) + (G-1)*(G-1) + (B-1)*(B-1);
	std::cout << "Error [Cyan]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Magenta]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(1,0,1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 1);
	PR_CHECK_NEARLY_EQ(G, 0);
	PR_CHECK_NEARLY_EQ(B, 1);

	const float q = (R-1)*(R-1) + (G-0)*(G-0) + (B-1)*(B-1);
	std::cout << "Error [Magenta]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Yellow]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(1,1,0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 1);
	PR_CHECK_NEARLY_EQ(G, 1);
	PR_CHECK_NEARLY_EQ(B, 0);

	const float q = (R-1)*(R-1) + (G-1)*(G-1) + (B-0)*(B-0);
	std::cout << "Error [Yellow]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Red]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(1,0,0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 1);
	PR_CHECK_NEARLY_EQ(G, 0);
	PR_CHECK_NEARLY_EQ(B, 0);

	const float q = (R-1)*(R-1) + (G-0)*(G-0) + (B-0)*(B-0);
	std::cout << "Error [Red]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Green]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(0,1,0);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 0);
	PR_CHECK_NEARLY_EQ(G, 1);
	PR_CHECK_NEARLY_EQ(B, 0);

	const float q = (R-0)*(R-0) + (G-1)*(G-1) + (B-0)*(B-0);
	std::cout << "Error [Green]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Blue]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(0,0,1);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 0);
	PR_CHECK_NEARLY_EQ(G, 0);
	PR_CHECK_NEARLY_EQ(B, 1);

	const float q = (R-0)*(R-0) + (G-0)*(G-0) + (B-1)*(B-1);
	std::cout << "Error [Blue]: " << q << std::endl;
}
PR_TEST("<-> sRGB [Custom]")
{
	PR::Spectrum spec = PR::RGBConverter::toSpec(0.8,0.5,0.2);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);
	PR_CHECK_NEARLY_EQ(R, 0.8);
	PR_CHECK_NEARLY_EQ(G, 0.5);
	PR_CHECK_NEARLY_EQ(B, 0.2);

	const float q = (R-0.8)*(R-0.8) + (G-0.5)*(G-0.5) + (B-0.2)*(B-0.2);
	std::cout << "Error [Custom]: " << q << std::endl;
}

PR_TEST("-> XYZ")
{
	PR::Spectrum spec;
	spec.fill(1);

	float X, Y;
	PR::XYZConverter::convert(spec, X, Y);
	PR_CHECK_NEARLY_EQ(X, 1/3.0f);
	PR_CHECK_NEARLY_EQ(Y, 1/3.0f);
}

PR_TEST("<-> XYZ")
{
	float x = 1/3.0f;
	PR::Spectrum spec = PR::XYZConverter::toSpec(x,x,x);
	float X, Y;
	PR::XYZConverter::convert(spec, X, Y);
	PR_CHECK_NEARLY_EQ(X, x);
	PR_CHECK_NEARLY_EQ(Y, x);
}
PR_TEST("<-> XYZ")
{
	float x = 0.4f;	float y = 0.2f;	float z = 0.4f;
	PR::Spectrum spec = PR::XYZConverter::toSpec(x,y,z);
	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ(X, x);
	PR_CHECK_NEARLY_EQ(Y, y);
	PR_CHECK_NEARLY_EQ(Z, z);
}
PR_TEST("<-> XYZ")
{
	float x = 0.2f;	float y = 0.2f;	float z = 0.4f;
	float b = x+y+z;
	PR::Spectrum spec = PR::XYZConverter::toSpec(x,y,z);
	float X, Y, Z;
	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR_CHECK_NEARLY_EQ(X, x/b);
	PR_CHECK_NEARLY_EQ(Y, y/b);
	PR_CHECK_NEARLY_EQ(Z, z/b);
}
PR_TEST("XYZ->RGB->XYZ")
{
	float X=1, Y=1, Z=1;
	float r,g,b;
	float x,y,z;
	PR::RGBConverter::fromXYZ(X,Y,Z,r,g,b);
	PR::RGBConverter::toXYZ(r,g,b,x,y,z);
	PR_CHECK_NEARLY_EQ(x, X);
	PR_CHECK_NEARLY_EQ(y, Y);
	PR_CHECK_NEARLY_EQ(z, Z);
}
PR_TEST("RGB->XYZ->RGB")
{
	float R=1, G=1, B=1;
	float r,g,b;
	float x,y,z;
	PR::RGBConverter::toXYZ(R,B,B,x,y,z);
	PR::RGBConverter::fromXYZ(x,y,z,r,b,g);
	PR_CHECK_NEARLY_EQ(r, R);
	PR_CHECK_NEARLY_EQ(g, B);
	PR_CHECK_NEARLY_EQ(b, G);
}

PR_END_TESTCASE()

PRT_BEGIN_MAIN
//PR::RGBConverter::init();
PRT_TESTCASE(Spectrum);
PRT_END_MAIN