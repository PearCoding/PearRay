#pragma once

#include "spectral/SpectralBlob.h"
#include "trace/IntersectionPoint.h"

namespace PR {
enum OutputSpectralEntryFlags {
	OSEF_Mono = 0x1
};

struct PR_LIB_CORE OutputSpectralEntry {
	Point2i Position;
	float MIS;
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

// Custom queue entries

template <typename T>
struct PR_LIB_CORE OutputCustomBaseEntry {
	Point2i Position;
	T Value;
};

struct PR_LIB_CORE OutputCustomSpectralEntry : public OutputCustomBaseEntry<SpectralBlob> {
	SpectralBlob Wavelengths;
	uint32 Flags;
	uint32 RayGroupID;
};

using OutputCustom3DEntry	   = OutputCustomBaseEntry<Vector3f>;
using OutputCustom1DEntry	   = OutputCustomBaseEntry<float>;
using OutputCustomCounterEntry = OutputCustomBaseEntry<uint32>;
} // namespace PR