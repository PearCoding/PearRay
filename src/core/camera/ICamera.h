#pragma once

#include "entity/ITransformable.h"
#include "spectral/SpectralBlob.h"

#include <optional>

namespace PR {
struct PR_LIB_CORE CameraSample {
	Size2i SensorSize;
	Point2f Pixel;
	Point2f Lens;
	uint32 PixelIndex;
	float Time;
	float BlendWeight;
	SpectralBlob Importance;
	SpectralBlob WavelengthNM;
	SpectralBlob WavelengthPDF;
};

struct PR_LIB_CORE CameraRay {
	Vector3f Origin	   = Vector3f::Zero();
	Vector3f Direction = Vector3f::Zero();
	float MinT		   = PR_EPSILON;
	float MaxT		   = PR_INF;
	bool IsMonochrome  = false;

	// Optional camera ray adaptations, if zero or negative, previous camera sample will be used
	float BlendWeight		   = 0;
	SpectralBlob Importance	   = SpectralBlob::Zero();
	SpectralBlob WavelengthNM  = SpectralBlob::Zero();
	SpectralBlob WavelengthPDF = SpectralBlob::Zero();
	float Time				   = 0;
};

class PR_LIB_CORE ICamera : public ITransformable {
public:
	ENTITY_CLASS

	ICamera(const std::string& name, const Transformf& transform);
	virtual ~ICamera();

	virtual std::optional<CameraRay> constructRay(const CameraSample& sample) const = 0;

	// This frame should be used as default initializer if applicable
	const static Vector3f DefaultDirection;
	const static Vector3f DefaultUp;
	const static Vector3f DefaultRight;
};
} // namespace PR
