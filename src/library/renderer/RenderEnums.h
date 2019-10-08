#pragma once

#include "PR_Config.h"

namespace PR {
enum SamplerMode {
	SM_RANDOM = 0,
	SM_UNIFORM,
	SM_JITTER,
	SM_MULTI_JITTER,
	SM_HALTON,
	SM_SOBOL
};

enum DebugMode {
	DM_DEPTH = 0,
	DM_NORMAL_BOTH,
	DM_NORMAL_POSITIVE,
	DM_NORMAL_NEGATIVE,
	DM_NORMAL_SPHERICAL,
	DM_TANGENT_BOTH,
	DM_TANGENT_POSITIVE,
	DM_TANGENT_NEGATIVE,
	DM_TANGENT_SPHERICAL,
	DM_BINORMAL_BOTH,
	DM_BINORMAL_POSITIVE,
	DM_BINORMAL_NEGATIVE,
	DM_BINORMAL_SPHERICAL,
	DM_UVW,
	DM_PDF,
	DM_EMISSION,
	DM_VALIDITY,
	DM_FLAG_INSIDE,
	DM_CONTAINER_ID,
	DM_BOUNDING_BOX
};

/* Visual feedback tile mode */
enum TileMode {
	TM_LINEAR = 0,
	TM_TILE,
	TM_SPIRAL
};

enum SpectralProcessMode {
	SPM_LINEAR = 0, // Each length will be used one after another
	SPM_SAMPLED
};

enum TimeMappingMode {
	TMM_CENTER = 0, // [0.5, 0.5]
	TMM_LEFT,		// [-1, 0]
	TMM_RIGHT		// [0, 1]
};

enum PPMGatheringMode {
	PGM_SPHERE = 0,
	PGM_DOME
};
} // namespace PR
