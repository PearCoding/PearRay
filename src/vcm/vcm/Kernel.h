#pragma once

#include "PR_Config.h"

namespace PR {
namespace VCM {

/// Kernel function used in gathering
inline float kernel(float nr2) { return 1 - nr2; }
inline float kernelarea(float R2) { return PR_PI * R2 / 2.0f; }

} // namespace VCM
} // namespace PR