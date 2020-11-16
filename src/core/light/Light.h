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

struct EntitySamplingInfo;
struct PR_LIB_CORE LightSampleInput {
	Vector4f RND;
	SpectralBlob WavelengthNM;
	const IntersectionPoint* Point		   = nullptr;
	const EntitySamplingInfo* SamplingInfo = nullptr;
	bool SamplePosition					   = false;
};

struct PR_LIB_CORE LightSampleOutput {
	SpectralBlob Radiance;
	Vector3f LightPosition; // If requested, position of light (abstract) surface
	LightPDF Position_PDF;
	float CosLight; // Cosinus between light normal and direction
	float Direction_PDF_S;
	Vector3f Outgoing; // Direction towards the light
};

class IInfiniteLight;
class IEntity;
class IEmission;
class RenderTileSession;
// Abstraction over infinite and area lights
class PR_LIB_CORE Light {
public:
	virtual ~Light() {}

	inline uint32 lightID() const { return mID; }

	std::string name() const;
	uint32 id() const; // TODO: Different name?
	uint32 entityID() const;
	uint32 infiniteLightID() const;

	inline bool isInfinite() const { return mEmission == nullptr; }
	bool hasDeltaDistribution() const;
	inline float relativeContribution() const { return mRelativeContribution; }

	void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const;
	void sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;
	LightPDF pdfPosition(const Vector3f& posOnLight, const EntitySamplingInfo* info = nullptr) const;
	// Returns pdf for direction respect to solid angle
	float pdfDirection(const Vector3f& dir, float cosLight = 1.0f) const;

	inline static Light makeInfLight(uint32 light_id, IInfiniteLight* infLight, float relContribution)
	{
		return Light(light_id, infLight, relContribution);
	}

	inline static Light makeAreaLight(uint32 light_id, IEntity* entity, IEmission* emission, float relContribution)
	{
		return Light(light_id, entity, emission, relContribution);
	}

	inline IEntity* asEntity() const { return reinterpret_cast<IEntity*>(mEntity); }
	inline IInfiniteLight* asInfiniteLight() const { return reinterpret_cast<IInfiniteLight*>(mEntity); }

private:
	Light(uint32 id, IInfiniteLight* infLight, float relContribution);
	Light(uint32 id, IEntity* entity, IEmission* emission, float relContribution);

	const uint32 mID;
	void* const mEntity;
	IEmission* const mEmission;
	const float mRelativeContribution;
};
} // namespace PR