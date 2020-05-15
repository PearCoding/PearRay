#pragma once

#include "spectral/ParametricBlob.h"

namespace DL {
class Data;
} // namespace DL

namespace PR {
class SpectralUpsampler;
class SpectralParser {
public:
	static ParametricBlob getSpectrum(SpectralUpsampler* upsampler, const DL::Data& data);
};
} // namespace PR
