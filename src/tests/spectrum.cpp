#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include <iostream>

// Not a automatic test!
int main(int argc, char** argv)
{
	PR::RGBConverter::init();

	float X, Y, Z;
	float x, y, z;
	float R, G, B;
	float gR, gG, gB;

	PR::Spectrum spec;
#ifndef PR_NO_SPECTRAL
	spec.setValueAtWavelength(640, 1);

	if (spec.value(56) != 1)
		std::cout << "FAILED" << std::endl;
#endif

	PR::XYZConverter::convertXYZ(spec, X, Y, Z);
	PR::XYZConverter::convert(spec, x, y, z);
	PR::RGBConverter::convert(spec, R, G, B);
	
	PR::RGBConverter::convert(spec, gR, gG, gB);
	PR::RGBConverter::gamma(gR, gG, gB);

	std::cout << "X " << X << " Y " << Y << " Z " << Z << std::endl;
	std::cout << "x " << x << " y " << y << " z " << z << std::endl;
	std::cout << "R " << R << " G " << G << " B " << B << std::endl;
	std::cout << "gR " << gR << " gG " << gG << " gB " << gB << std::endl;

	std::cout << (PR::Spectrum::fromBlackbody(5500).hasNaN() ? "Blackbody is invalid!" : "Blackbody ok.") << std::endl;

	PR::Spectrum candle = PR::Spectrum::fromBlackbody(1000);

	std::cout << "Blackbody Eq for T = 1000" << std::endl;
	std::cout << "380nm -> " << candle.value(3) << " should be nearly 0.54139" << std::endl;
	std::cout << "420nm -> " << candle.value(11) << " should be nearly 12.084" << std::endl;
	std::cout << "550nm -> " << candle.value(37) << " should be nearly 1.0307e+04" << std::endl;
	std::cout << "Max -> " << candle.max() << std::endl;

#if defined(PR_OS_WINDOWS) && defined(PR_DEBUG)
	system("pause");
#endif
	return 0;
}