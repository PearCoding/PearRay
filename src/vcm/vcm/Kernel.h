#pragma once

#include "PR_Config.h"

namespace PR {
namespace VCM {

/// Kernel function used in gathering
/*inline constexpr float kernel(float nr2) { return 1 - nr2; }
inline constexpr float kernelarea(float R2) { return PR_PI * R2 / 2.0f; }*/

inline constexpr float kernel(float) { return 1; }
inline constexpr float kernelarea(float R2) { return PR_PI * R2; }

} // namespace VCM
} // namespace PR