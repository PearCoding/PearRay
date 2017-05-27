#pragma once

#include "PR_Config.h"

namespace PR {
class Spectrum;
class PR_LIB IntensityConverter {
public:
	static void convert(const Spectrum& s, float& x, float& y, float& z);
};
}
