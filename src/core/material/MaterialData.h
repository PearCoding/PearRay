#pragma once

#include "MaterialContext.h"
#include "MaterialType.h"

namespace PR {

// Evaluation
struct PR_LIB_CORE MaterialEvalInput {
	MaterialEvalContext Context;
};

struct PR_LIB_CORE MaterialEvalOutput {
	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_CORE MaterialSampleInput {
	MaterialSampleContext Context;
	Vector2f RND;
};

struct PR_LIB_CORE MaterialSampleOutput {
	Vector3f L;

	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L);
	}
};
} // namespace PR