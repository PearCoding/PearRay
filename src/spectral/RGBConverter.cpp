#include "RGBConverter.h"
#include "Spectrum.h"

namespace PR
{
	void RGBConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		// TODO: This is not right! :D
		float avg = s.avg();
		x = avg;
		y = avg;
		z = avg;
	}
}