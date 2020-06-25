#pragma once

#include "trace/IntersectionPoint.h"
#include "spectral/SpectralBlob.h"

namespace PR {
enum OutputSpectralEntryFlags {
	OSEF_Mono = 0x1
};

struct PR_LIB_CORE OutputSpectralEntry {
	Point2i Position;
	SpectralBlob Weight;
	SpectralBlob Wavelengths;
	uint32 Flags;
	const uint32* Path;
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