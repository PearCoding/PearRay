#pragma once

#include "Light.h"
#include "math/Distribution1D.h"

namespace PR {

class Scene;

// Abstraction over infinite and area lights
class PR_LIB_CORE LightSampler {
public:
	LightSampler(Scene* scene);
	virtual ~LightSampler() {}

	inline const Light* sample(float rnd, float& pdf) const;
	inline const Light* sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const;

private:
	std::vector<Light> mLights;
	std::unique_ptr<Distribution1D> mSelector;
};
} // namespace PR

#include "LightSampler.inl"