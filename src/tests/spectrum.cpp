#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include "Test.h"

using namespace PR;

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

	for(int i = 0; i < PR::Spectrum::SAMPLING_COUNT; ++i)
		PR_CHECK_EQ(spec.value(i), 1);
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
	PR_CHECK_NEARLY_EQ(X, 1);
	PR_CHECK_NEARLY_EQ(Y, 1);
	PR_CHECK_NEARLY_EQ(Z, 1);
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

PR_END_TESTCASE()

PRT_BEGIN_MAIN
//PR::RGBConverter::init();
PRT_TESTCASE(Spectrum);
PRT_END_MAIN