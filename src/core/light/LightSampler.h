#pragma once

#include "Light.h"
#include "entity/IEntity.h"
#include "math/Distribution1D.h"

namespace PR {

class Scene;
class ISpectralMapper;

// Abstraction over infinite and area lights
class PR_LIB_CORE LightSampler {
public:
	using LightList = std::vector<std::unique_ptr<Light>>;

	LightSampler(Scene* scene, const SpectralRange& cameraSpectralRange);
	virtual ~LightSampler() {}

	inline const Light* sample(float rnd, float& pdf) const;
	inline std::pair<const Light*, float> sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;

	inline float pdfLightSelection(const Light* light) const;
	inline float pdfEntitySelection(const IEntity* entity) const;
	inline LightPDF pdfPosition(const IEntity* entity, const Vector3f& posOnLight, const EntitySamplingInfo* info = nullptr) const;
	inline float pdfDirection(const Vector3f& dir, const IEntity* entity, float cosLight = 1.0f) const;
	inline const Light* light(const IEntity* entity) const;

	inline const LightList& lights() const { return mLights; }
	inline const std::vector<Light*>& infiniteLights() const { return mInfLights; }

	inline size_t emissiveEntityCount() const { return mLightEntityMap.size(); }
	inline size_t lightCount() const { return mLights.size(); }

	inline float emissiveSurfaceArea() const { return mEmissiveSurfaceArea; }
	inline float emissiveSurfacePower() const { return mEmissiveSurfacePower; }
	inline float emissivePower() const { return mEmissivePower; }
	inline const SpectralRange& cameraSpectralRange() const { return mCameraSpectralRange; }
	inline const SpectralRange& lightSpectralRange() const { return mLightSpectralRange; }

	inline void setWavelengthSampler(const std::shared_ptr<ISpectralMapper>& mapper) { mWavelengthSampler = mapper; }
	inline ISpectralMapper* wavelengthSampler() const { return mWavelengthSampler.get(); }

private:
	using LightEntityMap = std::unordered_map<const IEntity*, size_t>;

	LightList mLights;
	LightEntityMap mLightEntityMap;
	std::vector<Light*> mInfLights; // Special purpose cache, as we expect inf lights be way less then area lights
	std::unique_ptr<Distribution1D> mSelector;
	float mInfLightSelectionProbability;
	float mEmissiveSurfaceArea;
	float mEmissiveSurfacePower;
	float mEmissivePower; // Including inf lights
	const SpectralRange mCameraSpectralRange;
	SpectralRange mLightSpectralRange;

	std::shared_ptr<ISpectralMapper> mWavelengthSampler;
};
} // namespace PR

#include "LightSampler.inl"