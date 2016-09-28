#include "IntensityConverter.h"
#include "Spectrum.h"

namespace PR
{
	void IntensityConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		float avg = s.luminousFlux();
		x = avg;
		y = avg;
		z = avg;
	}
}