#pragma once

#include "PR_Config.h"

namespace PR {
namespace VCM {
struct Options {
	size_t MaxCameraRayDepthHard = 16;
	size_t MaxCameraRayDepthSoft = 2;
	size_t MaxLightRayDepthHard	 = 8;
	size_t MaxLightRayDepthSoft	 = 2;
	// Only used if VCM is used with merging:
	size_t MaxLightSamples	 = 100000;
	float GatherRadiusFactor = 0.25f; // In respect to pixel area
	float ContractRatio		 = 0.4f;
	float SqueezeWeight2	 = 0.0f;
};
} // namespace VCM
} // namespace PR