#include "IntensityConverter.h"
#include "Spectrum.h"

namespace PR
{
	void IntensityConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		Spectrum t = s;
		t.normalize();
		float avg = t.avg();
		x = avg;
		y = avg;
		z = avg;
	}
}