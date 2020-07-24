#pragma once

#include "PR_Config.h"

namespace PR {
namespace Debug {
// Dump out image for development purposes
void PR_LIB_LOADER saveImage(const std::string& path, const float* data, size_t width, size_t height, size_t channels);
} // namespace Debug
} // namespace PR