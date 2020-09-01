#pragma once

#include "PR_Config.h"

namespace PR {
/* Cone defined by base center position, base radius and tip position
    * sectionCount gives the amount of "faces" used for triangulation

         Tip
          /\
         /  \
        /    \
       --------
         Base
    */
class PR_LIB_UI Cone {
public:
	static void triangulate(const Vector3f& basePos, const Vector3f& tipPos, float baseRadius, uint32 sectionCount, std::vector<float>& vertices);
	static void triangulateIndices(uint32 baseID, uint32 tipID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off = 2);
};
} // namespace PR