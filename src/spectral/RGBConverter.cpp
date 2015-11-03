#include "RGBConverter.h"
#include "Spectrum.h"

namespace PR
{
	void RGBConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		Spectrum spec = s;
		spec.normalize();

		float avg = s.avg();
		x = avg;
		y = avg;
		z = avg;
	}
}