#pragma once

#include "PR_Config.h"

namespace PR {
enum class MeshFeature : uint32 {
	Normal	 = 0x1,
	Texture	 = 0x2,
	Weight	 = 0x4,
	Velocity = 0x8,
	Material = 0x10
};
PR_MAKE_FLAGS(MeshFeature, MeshFeatures)

struct PR_LIB_CORE MeshInfo {
	MeshFeatures Features = 0;
	size_t NodeCount	  = 0;
	size_t TriangleCount  = 0;
	size_t QuadCount	  = 0;

	inline size_t faceCount() const { return TriangleCount + QuadCount; }
	inline bool isOnlyTriangular() const { return TriangleCount > 0 && QuadCount == 0; }
	inline bool isOnlyQuadrangular() const { return TriangleCount == 0 && QuadCount > 0; }
};
} // namespace PR