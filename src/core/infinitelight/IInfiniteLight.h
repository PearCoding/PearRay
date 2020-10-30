#pragma once

#include "entity/ITransformable.h"
#include "trace/IntersectionPoint.h"

namespace PR {
struct PR_LIB_CORE InfiniteLightEvalInput {
	const IntersectionPoint* Point = nullptr; // Can be null if not called from a surface
	PR::Ray Ray;
};

struct PR_LIB_CORE InfiniteLightEvalOutput {
	SpectralBlob Radiance;
	float PDF_S;
};

struct PR_LIB_CORE InfiniteLightSampleInput {
	Vector4f RND;
	SpectralBlob WavelengthNM;
	const IntersectionPoint* Point = nullptr;
	bool SamplePosition			   = false;
};

struct PR_LIB_CORE InfiniteLightSampleOutput {
	SpectralBlob Radiance;
	float PDF_S;
	Vector3f Position;
	Vector3f Outgoing;
};

class RenderTileSession;
class PR_LIB_CORE IInfiniteLight : public ITransformable {
public:
	IInfiniteLight(uint32 id, const std::string& name);
	virtual ~IInfiniteLight() {}

	virtual bool hasDeltaDistribution() const { return false; }

	virtual void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out, const RenderTileSession& session) const = 0;
	/*
		Sample the light with point information.
	*/
	virtual void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out, const RenderTileSession& session) const = 0;

	virtual float power() const = 0; // Average (W/m^2)

	virtual std::string dumpInformation() const;
};
} // namespace PR
