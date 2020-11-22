#pragma once

#include "spectral/SpectralBlob.h"
#include "trace/IntersectionPoint.h"

namespace PR {
enum OutputSpectralEntryFlags {
	OSEF_Mono = 0x1
};

class PR_LIB_CORE OutputSpectralEntry {
public:
	Point2i Position;
	SpectralBlob MIS;
	SpectralBlob Importance;
	SpectralBlob Radiance;
	SpectralBlob Wavelengths;
	uint32 Flags;
	uint32 RayGroupID;
	const uint32* Path;

	inline SpectralBlob contribution() const { return MIS * Importance * Radiance; }
};

struct PR_LIB_CORE OutputShadingPointEntry {
	Point2i Position;
	IntersectionPoint SP;
	const uint32* Path;
};

struct PR_LIB_CORE OutputFeedbackEntry {
	Point2i Position;
	uint32 Feedback;
};
} // namespace PR