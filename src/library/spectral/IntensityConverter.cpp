#include "IntensityConverter.h"
#include "Spectrum.h"

namespace PR {
void IntensityConverter::convert(const Spectrum& s, float& x, float& y, float& z)
{
	float avg = static_cast<float>(s.luminousFlux());
	x		  = avg;
	y		  = avg;
	z		  = avg;
}
}
