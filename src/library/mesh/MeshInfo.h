#pragma once

#include "PR_Config.h"

namespace PR {
enum MeshFeatures : uint32 {
	MF_HAS_UV		= 0x1,
	MF_HAS_VELOCITY = 0x2,
	MF_HAS_MATERIAL = 0x4
};

struct PR_LIB MeshInfo {
	uint32 Features		 = 0;
	size_t NodeCount	 = 0;
	size_t TriangleCount = 0;
	size_t QuadCount	 = 0;

	inline size_t faceCount() const { return TriangleCount + QuadCount; }
	inline bool isOnlyTriangular() const { return TriangleCount > 0 && QuadCount == 0; }
	inline bool isOnlyQuadrangular() const { return TriangleCount == 0 && QuadCount > 0; }
};
} // namespace PR