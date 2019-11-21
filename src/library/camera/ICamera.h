#pragma once

#include "entity/ITransformable.h"
#include "ray/RayPackage.h"

namespace PR {
struct PR_LIB CameraSample {
	Vector2i SensorSize; // Full Size (Width and Height)
	float Pixel[2];
	float R[2];
	uint32 PixelIndex;
	float Time;
	float Weight;
	uint32 WavelengthIndex;
};

class PR_LIB ICamera : public ITransformable {
public:
	ENTITY_CLASS

	ICamera(uint32 id, const std::string& name);
	virtual ~ICamera();

	virtual Ray constructRay(const CameraSample& sample) const = 0;
};
} // namespace PR
