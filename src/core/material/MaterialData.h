#pragma once

#include "MaterialContext.h"
#include "MaterialType.h"
#include "Random.h"
#include "shader/ShadingContext.h"

namespace PR {

struct PR_LIB_CORE MaterialBaseOutput {
	SpectralBlob PDF_S;
	MaterialSampleFlags Flags = 0;

	inline bool isNull() const { return Flags & MaterialSampleFlag::Null; }
	inline bool isDelta() const { return Flags & MaterialSampleFlag::DeltaDistribution; }
	inline bool isSpectralVarying() const { return Flags & MaterialSampleFlag::SpectralVarying; }
	inline bool isSpatialVarying() const { return Flags & MaterialSampleFlag::SpatialVarying; }
	inline bool isTimeVarying() const { return Flags & MaterialSampleFlag::TimeVarying; }
	inline bool isFlourescent() const { return Flags & MaterialSampleFlag::Flourescent; }

	// When to reduce to hero wavelength only
	inline bool isHeroCollapsing() const { return isDelta() && isSpectralVarying(); }
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
	MaterialScatteringType Type = MaterialScatteringType::DiffuseReflection;

	inline Vector3f globalL(const IntersectionPoint& ip) const
	{
		return Tangent::fromTangentSpace(ip.Surface.N, ip.Surface.Nx, ip.Surface.Ny, L).normalized();
	}

	static inline MaterialSampleOutput Reject(MaterialScatteringType type = MaterialScatteringType::DiffuseReflection, uint8 flags = 0)
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