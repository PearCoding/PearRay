#pragma once

#include "PR_Config.h"

namespace PR {
namespace Triangulation {
struct PR_LIB_BASE Triangle {
	uint32 V0;
	uint32 V1;
	uint32 V2;
};

/// Simple 2d triangulation based on delaunay triangulation
std::vector<Triangle> PR_LIB_BASE triangulate2D(const std::vector<Vector2f>& vertices);

/// 3d triangulation based on extended delaunay triangulation with optional surface extraction
std::vector<Triangle> PR_LIB_BASE triangulate3D(const std::vector<Vector3f>& vertices, bool surfaceOnly = true);

} // namespace Triangulation
} // namespace PR
