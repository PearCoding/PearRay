#pragma once

#include "IMaterialType.h"
#include "shader/ShadingPoint.h"

namespace PR {

// Evaluation
struct PR_LIB_INLINE MaterialEvalInput {
	ShadingPoint Point;
	float NdotL;
	Vector3f Outgoing;
};

struct PR_LIB_INLINE MaterialEvalOutput {
	ColorTriplet Weight;
	float PDF_S;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_INLINE MaterialSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB_INLINE MaterialSampleOutput {
	Vector3f Outgoing;
	ColorTriplet Weight;
	float PDF_S;
	MaterialScatteringType Type;
};
} // namespace PR