#pragma once

#include "trace/IntersectionPoint.h"

namespace PR {
struct LightPDF {
	float Value;
	bool IsArea;
};
struct PR_LIB_CORE LightEvalInput {
	const IntersectionPoint* Point = nullptr; // Can be null if nothing was hit
	PR::Ray Ray;
};

struct PR_LIB_CORE LightEvalOutput {
	SpectralBlob Radiance;
	LightPDF PDF;
};

struct PR_LIB_CORE LightSampleInput {
	Vector4f RND;
	SpectralBlob WavelengthNM;
};

struct PR_LIB_CORE LightSampleOutput {
	SpectralBlob Radiance;
	LightPDF PDF;
	Vector3f Outgoing;
};

class IInfiniteLight;
class IEntity;
class IEmission;
class RenderTileSession;
// Abstraction over infinite and area lights
class PR_LIB_CORE Light {
public:
	Light(IInfiniteLight* infLight);
	Light(IEntity* entity, IEmission* emission);
	virtual ~Light() {}

	inline bool isInfinite() const { return mEmission == nullptr; }
	inline bool isHit(IEntity* entity) const { return (isInfinite() && entity == nullptr) || (!isInfinite() && entity == mEntity); }
	bool hasDeltaDistribution() const;

	void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const;
	void sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;

private:
	void* const mEntity;
	IEmission* const mEmission;
};
} // namespace PR