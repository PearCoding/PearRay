#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_UI Cylinder {
public:
	static void triangulate(const Vector3f& base1Pos, const Vector3f& base2Pos, float topRadius, float bottomRadius, uint32 sectionCount, std::vector<float>& indices);
	static void triangulateIndices(uint32 base1ID, uint32 base2ID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off = 2);
};
} // namespace PR