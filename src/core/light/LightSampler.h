#pragma once

#include "Light.h"
#include "math/Distribution1D.h"

namespace PR {

class Scene;

// Abstraction over infinite and area lights
class PR_LIB_CORE LightSampler {
public:
	using LightList = std::vector<std::unique_ptr<Light>>;

	LightSampler(Scene* scene);
	virtual ~LightSampler() {}

	inline const Light* sample(float rnd, float& pdf) const;
	inline const Light* sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;

	inline const LightList& lights() const { return mLights; }

	inline float emissiveSurfaceArea() const { return mEmissiveSurfaceArea; }
	inline float emissiveSurfacePower() const { return mEmissiveSurfacePower; }
	inline float emissivePower() const { return mEmissivePower; }

private:
	LightList mLights;
	std::unique_ptr<Distribution1D> mSelector;
	float mEmissiveSurfaceArea;
	float mEmissiveSurfacePower;
	float mEmissivePower; // Including inf lights
};
} // namespace PR

#include "LightSampler.inl"