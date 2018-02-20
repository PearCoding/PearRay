#pragma once

namespace PR {
enum SamplerMode {
	SM_Random,
	SM_Uniform,
	SM_Jitter,
	SM_MultiJitter,
	SM_HaltonQMC
};

enum DebugMode {
	DM_None,
	DM_Depth,
	DM_Normal_Both,
	DM_Normal_Positive,
	DM_Normal_Negative,
	DM_Normal_Spherical,
	DM_Tangent_Both,
	DM_Tangent_Positive,
	DM_Tangent_Negative,
	DM_Tangent_Spherical,
	DM_Binormal_Both,
	DM_Binormal_Positive,
	DM_Binormal_Negative,
	DM_Binormal_Spherical,
	DM_UVW,
	DM_PDF,
	DM_Emission,
	DM_Validity,
	DM_Flag_Inside,
	DM_Container_ID
};

enum IntegratorMode {
	IM_Direct,
	IM_BiDirect,
	IM_PPM // Progressive Photon Mapping
};

/* Visual feedback tile mode */
enum TileMode {
	TM_Linear,
	TM_Tile,
	TM_Spiral
};

enum TimeMappingMode {
	TMM_Center, // [0.5, 0.5]
	TMM_Left,   // [-1, 0]
	TMM_Right   // [0, 1]
};

enum PPMGatheringMode {
	PGM_Sphere,
	PGM_Dome
};
}
