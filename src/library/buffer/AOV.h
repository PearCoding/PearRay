#pragma once

namespace PR {
enum AOV3D {
	AOV_Position = 0,
	AOV_Normal,
	AOV_NormalG,
	AOV_Tangent,
	AOV_Bitangent,
	AOV_View,
	AOV_UVW,
	AOV_DPDT,

	AOV_3D_COUNT
};

enum AOV1D {
	AOV_EntityID = 0,
	AOV_MaterialID,
	AOV_EmissionID,
	AOV_DisplaceID,
	AOV_Depth,
	AOV_Time,

	AOV_1D_COUNT
};

enum AOVCounter {
	AOV_SampleCount = 0,
	AOV_Feedback,

	AOV_COUNTER_COUNT
};
} // namespace PR