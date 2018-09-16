#pragma once

#include "entity/VirtualEntity.h"
#include "math/SIMD.h"

namespace PR {
class RayPackage;

struct CameraSample {
	Eigen::Vector2i SensorSize; // Full Size (Width and Height)
	vfloat Pixel[2];
	vfloat R[2];
	vuint32 PixelIndex;
	vfloat Time;
	vuint32 WavelengthIndex;
};

class PR_LIB ICamera : public VirtualEntity {
public:
	ENTITY_CLASS

	ICamera(uint32 id, const std::string& name);
	virtual ~ICamera();

	virtual RayPackage constructRay(const CameraSample& sample) const = 0;
};
} // namespace PR
