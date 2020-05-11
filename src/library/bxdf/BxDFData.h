#pragma once

#include "BxDFType.h"
#include "shader/ShadingPoint.h"

namespace PR {

// Evaluation
struct PR_LIB BxDFEvalInput {
	Vector3f* Outgoing;
};

struct PR_LIB BxDFEvalOutput {
	SpectralBlob* Weight;
	float* PDF_S;
};

struct PR_LIB BxDFEval {
	size_t Size;
	BxDFEvalInput In;
	BxDFEvalOutput Out;
};

// Sampling
struct PR_LIB BxDFSampleInput {
	Vector2f* RND;
};

struct PR_LIB BxDFSampleOutput {
	Vector3f* Outgoing;
	SpectralBlob* Weight;
	float* PDF_S;
	BxDFType* Type;
};

struct PR_LIB BxDFSample {
	size_t Size;
	BxDFSampleInput In;
	BxDFSampleOutput Out;
};

} // namespace PR