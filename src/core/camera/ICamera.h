#pragma once

#include "entity/ITransformable.h"
#include "ray/Ray.h"

namespace PR {
struct PR_LIB_CORE CameraSample {
	Size2i SensorSize;
	Point2f Pixel;
	Point2f Lens;
	uint32 PixelIndex;
	float Time;
	SpectralBlob Weight;
	SpectralBlob WavelengthNM;
};

class PR_LIB_CORE ICamera : public ITransformable {
public:
	ENTITY_CLASS

	ICamera(uint32 id, const std::string& name);
	virtual ~ICamera();

	virtual Ray constructRay(const CameraSample& sample) const = 0;

	// This frame should be used as default initializer if applicable
	const static Vector3f DefaultDirection;
	const static Vector3f DefaultUp;
	const static Vector3f DefaultRight;
};
} // namespace PR
