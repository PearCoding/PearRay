#pragma once

#include "MaterialType.h"
#include "shader/ShadingPoint.h"

namespace PR {

// Evaluation
struct PR_LIB MaterialEvalInput {
	ShadingPoint Point;
	float NdotL;
	Vector3f Outgoing;
};

struct PR_LIB MaterialEvalOutput {
	ColorTriplet Weight;
	float PDF_S;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB MaterialSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB MaterialSampleOutput {
	Vector3f Outgoing;
	ColorTriplet Weight;
	float PDF_S;
	MaterialScatteringType Type;
};
} // namespace PR