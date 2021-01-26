#pragma once

#include "Random.h"
#include "spectral/SpectralRange.h"
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
	Random& RND;
	SpectralBlob WavelengthNM; // Will be used if SampleWavelength is false
	const IntersectionPoint* Point		   = nullptr;
	const EntitySamplingInfo* SamplingInfo = nullptr;
	bool SamplePosition					   = false;
	bool SampleWavelength				   = false;

	inline explicit LightSampleInput(Random& rnd)
		: RND(rnd)
	{
	}
};

struct PR_LIB_CORE LightSampleOutput {
	SpectralBlob Radiance;
	SpectralBlob WavelengthNM;	 // If requested with SampleWavelength, or same as input
	SpectralBlob Wavelength_PDF; // If requested, with SampleWavelength,  or 1
	Vector3f LightPosition;		 // If requested, position of light (abstract) surface
	LightPDF Position_PDF;
	float CosLight; // Cosinus between light normal and direction
	float Direction_PDF_S;
	Vector3f Outgoing; // Direction towards the light
};

class IInfiniteLight;
class IEntity;
class IEmission;
class LightSampler;
class RenderTileSession;
// Abstraction over infinite and area lights
class PR_LIB_CORE Light {
public:
	virtual ~Light() {}

	inline uint32 lightID() const { return mID; }

	std::string name() const;

	inline bool isInfinite() const { return mEmission == nullptr; }
	bool hasDeltaDistribution() const;
	inline float relativeContribution() const { return mRelativeContribution; }

	SpectralBlob averagePower(const SpectralBlob& wavelengths) const;
	/// Returns wavelength range this light emits in. If unbounded, the framework will use the global spectral domain
	SpectralRange spectralRange() const;

	void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const;
	void sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;
	LightPDF pdfPosition(const Vector3f& posOnLight, const EntitySamplingInfo* info = nullptr) const;
	// Returns pdf for direction respect to solid angle
	float pdfDirection(const Vector3f& dir, float cosLight = 1.0f) const;

	inline static Light makeInfLight(uint32 light_id, IInfiniteLight* infLight, float relContribution, const LightSampler* sampler)
	{
		return Light(light_id, infLight, relContribution, sampler);
	}

	inline static Light makeAreaLight(uint32 light_id, IEntity* entity, IEmission* emission, float relContribution, const LightSampler* sampler)
	{
		return Light(light_id, entity, emission, relContribution, sampler);
	}

	inline IEntity* asEntity() const { return reinterpret_cast<IEntity*>(mEntity); }
	inline IInfiniteLight* asInfiniteLight() const { return reinterpret_cast<IInfiniteLight*>(mEntity); }

private:
	Light(uint32 id, IInfiniteLight* infLight, float relContribution, const LightSampler* sampler);
	Light(uint32 id, IEntity* entity, IEmission* emission, float relContribution, const LightSampler* sampler);

	const uint32 mID;
	void* const mEntity;
	IEmission* const mEmission;
	const float mRelativeContribution;
	const LightSampler* mSampler;
};
} // namespace PR