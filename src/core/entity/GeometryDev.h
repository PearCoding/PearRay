#pragma once

#include "PR_Config.h"

#include <embree3/rtcore.h>

namespace PR {

// Structure to hide embree header from other parts of the engine except the actual entity implementation and the scene structure
struct PR_LIB_CORE GeometryDev {
	RTCDevice Device;

	inline explicit GeometryDev(const RTCDevice& dev)
		: Device(dev)
	{
	}

	inline operator RTCDevice() const { return Device; }
};
} // namespace PR