#pragma once

#include "EmissionContext.h"
#include "Random.h"
#include "shader/ShadingContext.h"

namespace PR {

// Evaluation
struct PR_LIB_CORE EmissionEvalInput {
	EmissionEvalContext Context;
	PR::ShadingContext ShadingContext;
};

struct PR_LIB_CORE EmissionPDFOutput {
	SpectralBlob PDF_S;
};

struct PR_LIB_CORE EmissionEvalOutput {
	SpectralBlob PDF_S;
	SpectralBlob Radiance;
};

// Sampling
struct PR_LIB_CORE EmissionSampleInput {
	EmissionSampleContext Context;
	PR::ShadingContext ShadingContext;
	Random& RND;

	inline EmissionSampleInput(Random& rnd)
		: RND(rnd)
	{
	}

	inline EmissionSampleInput(const IntersectionPoint& ip, uint32 thread_index, Random& rnd)
		: Context(EmissionSampleContext::fromIP(ip))
		, ShadingContext(ShadingContext::fromIP(thread_index, ip))
		, RND(rnd)
	{
	}
};
struct PR_LIB_CORE EmissionSampleOutput {
	ShadingVector L;
	SpectralBlob PDF_S; // In respect to the radiance

	inline Vector3f globalL(const Vector3f& N, const Vector3f& Nx, const Vector3f& Ny) const
	{
		return Tangent::fromTangentSpace(N, Nx, Ny, L).normalized();
	}

	static inline EmissionSampleOutput Reject()
	{
		EmissionSampleOutput out;
		out.L			 = Vector3f::Zero();
		out.PDF_S		 = SpectralBlob::Zero();
		return out;
	}
};
} // namespace PR
