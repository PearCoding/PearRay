#pragma once

#include "spectral/Spectrum.h"

namespace DL {
class Data;
} // namespace DL

namespace PR {
class SpectralParser {
public:
	static Spectrum getSpectrum(const std::shared_ptr<SpectrumDescriptor>& desc, const DL::Data& data);
};
} // namespace PR
