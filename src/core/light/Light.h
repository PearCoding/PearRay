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
	const IntersectionPoint* Point = nullptr;
	bool SamplePosition			   = false;
};

struct PR_LIB_CORE LightSampleOutput {
	SpectralBlob Radiance;
	LightPDF PDF;
	Vector3f Position;
	Vector3f Outgoing; // Direction towards the light (and if given -> Position)
};

class IInfiniteLight;
class IEntity;
class IEmission;
class RenderTileSession;
// Abstraction over infinite and area lights
class PR_LIB_CORE Light {
public:
	virtual ~Light() {}

	std::string name() const;

	inline bool isInfinite() const { return mEmission == nullptr; }
	inline bool isHit(IEntity* entity) const { return (isInfinite() && entity == nullptr) || (!isInfinite() && entity == mEntity); }
	bool hasDeltaDistribution() const;
	inline float relativeContribution() const { return mRelativeContribution; }

	void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const;
	void sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;

	inline static Light makeInfLight(IInfiniteLight* infLight, float relContribution)
	{
		return Light(infLight, relContribution);
	}

	inline static Light makeAreaLight(IEntity* entity, IEmission* emission, float relContribution)
	{
		return Light(entity, emission, relContribution);
	}

private:
	Light(IInfiniteLight* infLight, float relContribution);
	Light(IEntity* entity, IEmission* emission, float relContribution);

	void* const mEntity;
	IEmission* const mEmission;
	const float mRelativeContribution;
};
} // namespace PR