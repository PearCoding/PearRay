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
enum class EmissionSampleRequest {
	Direction  = 0x1,
	Wavelength = 0x2
};
PR_MAKE_FLAGS(EmissionSampleRequest, EmissionSampleRequests)

struct PR_LIB_CORE EmissionSampleInput {
	EmissionSampleContext Context;
	PR::ShadingContext ShadingContext;
	Random& RND;

	EmissionSampleRequests Requests = EmissionSampleRequest::Direction;

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
	ShadingVector L;		   // Only valid if EmissionSampleRequest::Direction is set
	SpectralBlob WavelengthNM; // Only valid if EmissionSampleRequest::Wavelength is set

	SpectralBlob PDF_S; // In respect to the radiance

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L).normalized();
	}

	static inline EmissionSampleOutput Reject()
	{
		EmissionSampleOutput out;
		out.L			 = Vector3f::Zero();
		out.PDF_S		 = SpectralBlob::Zero();
		out.WavelengthNM = SpectralBlob::Zero();
		return out;
	}
};
} // namespace PR
