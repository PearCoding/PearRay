#pragma once

#include "PR_Config.h"

namespace PR {
namespace Delaunay {
struct PR_LIB_BASE Triangle {
	uint32 V0;
	uint32 V1;
	uint32 V2;
};

// Simple 2d triangulation
std::vector<Triangle> PR_LIB_BASE triangulate2D(const std::vector<Vector2f>& vertices);

} // namespace Delaunay
} // namespace PR