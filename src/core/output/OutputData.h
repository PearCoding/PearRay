#pragma once

#include "Feedback.h"
#include "spectral/SpectralBlob.h"
#include "trace/IntersectionPoint.h"

namespace PR {
enum class OutputSpectralEntryFlag : uint32 {
	Mono = 0x1
};
PR_MAKE_FLAGS(OutputSpectralEntryFlag, OutputSpectralEntryFlags)

struct PR_LIB_CORE OutputSpectralEntry {
	Point2i Position;
	SpectralBlob MIS;
	SpectralBlob Importance;
	SpectralBlob Radiance;
	SpectralBlob Wavelengths;
	OutputSpectralEntryFlags Flags;
	uint32 RayGroupID;
	const uint32* Path;

	inline SpectralBlob contribution() const { return MIS * Importance * Radiance; }
	inline float contribution(size_t index) const { return MIS[index] * Importance[index] * Radiance[index]; }
};

struct PR_LIB_CORE OutputShadingPointEntry {
	Point2i Position;
	IntersectionPoint SP;
	const uint32* Path;
};

struct PR_LIB_CORE OutputFeedbackEntry {
	Point2i Position;
	OutputFeedbackFlags Feedback;
};

// Custom queue entries

template <typename T>
struct PR_LIB_CORE OutputCustomBaseEntry {
	Point2i Position;
	T Value;
};

struct PR_LIB_CORE OutputCustomSpectralEntry : public OutputCustomBaseEntry<SpectralBlob> {
	SpectralBlob Wavelengths;
	OutputSpectralEntryFlags Flags;
	uint32 RayGroupID;
};

using OutputCustom3DEntry	   = OutputCustomBaseEntry<Vector3f>;
using OutputCustom1DEntry	   = OutputCustomBaseEntry<float>;
using OutputCustomCounterEntry = OutputCustomBaseEntry<uint32>;
} // namespace PR