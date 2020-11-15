#pragma once

#include "PR_Config.h"

namespace PR {
namespace VCM {
struct Options {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
};
} // namespace VCM
} // namespace PR