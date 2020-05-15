#pragma once

#include "spectral/SpectralUpsampler.h"

namespace PR {
PR_LIB_LOADER std::shared_ptr<SpectralUpsampler> loadDefaultSpectralUpsampler();
}