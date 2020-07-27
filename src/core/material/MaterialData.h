#pragma once

#include "MaterialContext.h"
#include "MaterialType.h"
#include "shader/ShadingContext.h"

namespace PR {

// Evaluation
struct PR_LIB_CORE MaterialEvalInput {
	MaterialEvalContext Context;
	PR::ShadingContext ShadingContext;
};

struct PR_LIB_CORE MaterialEvalOutput {
	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_CORE MaterialSampleInput {
	MaterialSampleContext Context;
	PR::ShadingContext ShadingContext;
	Vector2f RND;
};

struct PR_LIB_CORE MaterialSampleOutput {
	Vector3f L;

	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L).normalized();
	}
};
} // namespace PR