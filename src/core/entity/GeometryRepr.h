#pragma once

#include "PR_Config.h"

#include <embree3/rtcore.h>

namespace PR {

// Structure to hide embree header from other parts of the engine except the actual entity implementation and the scene structure
struct PR_LIB_CORE GeometryRepr {
	RTCGeometry Geometry;

	inline explicit GeometryRepr(const RTCGeometry& geom)
		: Geometry(geom)
	{
	}

	inline operator RTCGeometry() const { return Geometry; }
};
} // namespace PR