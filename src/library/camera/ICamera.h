#pragma once

#include "entity/ITransformable.h"
#include "ray/RayPackage.h"

namespace PR {
struct PR_LIB CameraSample {
	Size2i SensorSize;
	Point2f Pixel;
	Point2f Lens;
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
