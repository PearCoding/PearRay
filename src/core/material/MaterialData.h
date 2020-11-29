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

struct PR_LIB_CORE MaterialPDFOutput {
	SpectralBlob PDF_S;
};

struct PR_LIB_CORE MaterialEvalOutput : public MaterialPDFOutput {
	SpectralBlob Weight;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_CORE MaterialSampleInput {
	MaterialSampleContext Context;
	PR::ShadingContext ShadingContext;
	Vector2f RND;
};

enum MaterialScatterFlags {
	MSF_DeltaDistribution = 0x1,
	MSF_SpectralVarying	  = 0x2 // The sampled direction (not the weight) is different for each wavelength
};

struct PR_LIB_CORE MaterialSampleOutput {
	ShadingVector L;

	SpectralBlob Weight;
	SpectralBlob PDF_S;
	MaterialScatteringType Type;
	uint8 Flags = 0;

	inline bool isDelta() const { return Flags & MSF_DeltaDistribution; }
	inline bool isSpectralVarying() const { return Flags & MSF_SpectralVarying; }

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L).normalized();
	}

	static inline MaterialSampleOutput Reject(MaterialScatteringType type = MST_DiffuseReflection, uint8 flags = 0)
	{
		return MaterialSampleOutput{ ShadingVector(Vector3f::Zero()),
									 SpectralBlob::Zero(),
									 SpectralBlob::Zero(),
									 type, flags};
	}
};
} // namespace PR