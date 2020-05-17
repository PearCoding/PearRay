#pragma once

#include "spectral/SpectralUpsampler.h"

namespace PR {
class PR_LIB_LOADER DefaultSRGB {
public:
	static std::shared_ptr<SpectralUpsampler> loadSpectralUpsampler();
};
} // namespace PR