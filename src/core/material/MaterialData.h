#pragma once

#include "MaterialContext.h"
#include "MaterialType.h"
#include "Random.h"
#include "shader/ShadingContext.h"

namespace PR {

enum MaterialScatterFlags {
	MSF_Null			  = 0x1,
	MSF_DeltaDistribution = 0x2,
	MSF_SpectralVarying	  = 0x4,  // TODO
	MSF_SpatialVarying	  = 0x8,  // TODO
	MSF_TimeVarying		  = 0x10, // TODO
};

struct PR_LIB_CORE MaterialBaseOutput {
	SpectralBlob PDF_S;
	uint8 Flags = 0;

	inline bool isNull() const { return Flags & MSF_Null; }
	inline bool isDelta() const { return Flags & MSF_DeltaDistribution; }
	inline bool isSpectralVarying() const { return Flags & MSF_SpectralVarying; }
	inline bool isSpatialVarying() const { return Flags & MSF_SpatialVarying; }
	inline bool isTimeVarying() const { return Flags & MSF_TimeVarying; }

	// When to reduce to hero wavelength only
	inline bool isHeroCollapsing() const { return !isNull() && isDelta(); }
};

// Evaluation
struct PR_LIB_CORE MaterialEvalInput {
	MaterialEvalContext Context;
	PR::ShadingContext ShadingContext;
};

struct PR_LIB_CORE MaterialPDFOutput : public MaterialBaseOutput {
};

struct PR_LIB_CORE MaterialEvalOutput : public MaterialBaseOutput {
	SpectralBlob Weight;
	MaterialScatteringType Type;
};

// Sampling
struct PR_LIB_CORE MaterialSampleInput {
	MaterialSampleContext Context;
	PR::ShadingContext ShadingContext;
	Random& RND;

	inline MaterialSampleInput(Random& rnd)
		: RND(rnd)
	{
	}

	inline MaterialSampleInput(const IntersectionPoint& ip, uint32 thread_index, Random& rnd)
		: Context(MaterialSampleContext::fromIP(ip))
		, ShadingContext(ShadingContext::fromIP(thread_index, ip))
		, RND(rnd)
	{
	}
};
struct PR_LIB_CORE MaterialSampleOutput : public MaterialBaseOutput {
	ShadingVector L;

	SpectralBlob Weight;
	MaterialScatteringType Type;

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L).normalized();
	}

	static inline MaterialSampleOutput Reject(MaterialScatteringType type = MST_DiffuseReflection, uint8 flags = 0)
	{
		MaterialSampleOutput out;
		out.L	   = Vector3f::Zero();
		out.Weight = SpectralBlob::Zero();
		out.PDF_S  = SpectralBlob::Zero();
		out.Type   = type;
		out.Flags  = flags;
		return out;
	}
};
} // namespace PR