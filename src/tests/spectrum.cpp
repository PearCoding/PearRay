#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include <iostream>

int main(int argc, char** argv)
{
	PR::XYZConverter::init();
	PR::RGBConverter::init();

	float X, Y, Z;
	float x, y, z;
	float R, G, B;
	float gR, gG, gB;

	PR::Spectrum spec;
	spec.setValueAtWavelength(640, 1);

	if (spec.value(56) != 1)
	{
		std::cout << "FAILED" << std::endl;
	}

	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR::XYZConverter::convert(spec, x, y, z);
	PR::RGBConverter::convert(spec, R, G, B);
	PR::RGBConverter::convertGAMMA(spec, gR, gG, gB);

	std::cout << "X " << X << " Y " << Y << " Z " << Z << std::endl;
	std::cout << "x " << x << " y " << y << " z " << z << std::endl;
	std::cout << "R " << R << " G " << G << " B " << B << std::endl;
	std::cout << "gR " << gR << " gG " << gG << " gB " << gB << std::endl;

#if defined(PR_OS_WINDOWS) && defined(PR_DEBUG)
	system("pause");
#endif
	return 0;
}