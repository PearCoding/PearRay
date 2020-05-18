#pragma once

#include "MaterialType.h"
#include "shader/ShadingPoint.h"

namespace PR {

// Evaluation
struct PR_LIB_CORE MaterialEvalInput {
	ShadingPoint Point;
	float NdotL;
	Vector3f Outgoing;
};

struct PR_LIB_CORE MaterialEvalOutput {
	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_CORE MaterialSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB_CORE MaterialSampleOutput {
	Vector3f Outgoing;
	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;
};
} // namespace PR