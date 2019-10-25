#pragma once

#include "IMaterialType.h"
#include "shader/ShadingPoint.h"

namespace PR {

// Evaluation
struct PR_LIB_INLINE MaterialEvalInput {
	ShadingPoint Point;
	Vector3f Outgoing;
};

struct PR_LIB_INLINE MaterialEvalOutput {
	float Weight;
	float PDF_S_Forward;
	float PDF_S_Backward;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_INLINE MaterialSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB_INLINE MaterialSampleOutput {
	Vector3f Outgoing;
	float Weight;
	float PDF_S_Forward;
	float PDF_S_Backward;
	MaterialScatteringType Type;
};
} // namespace PR