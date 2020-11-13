#pragma once

#include "PR_Config.h"

namespace PR {
constexpr size_t AR_SPECTRAL_BANDS = 11; // The standard (v.1.4) has 11 bins
constexpr float AR_SPECTRAL_DELTA  = 40;
constexpr float AR_SPECTRAL_START  = 320;
constexpr float AR_SPECTRAL_END	   = AR_SPECTRAL_START + AR_SPECTRAL_BANDS * AR_SPECTRAL_DELTA;

constexpr size_t RES_AZ = 512;
constexpr size_t RES_EL = 256;
} // namespace PR