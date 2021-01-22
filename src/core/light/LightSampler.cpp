#include "LightSampler.h"
#include "Logger.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "scene/Scene.h"
#include "scene/SceneDatabase.h"

namespace PR {

LightSampler::LightSampler(Scene* scene)
	: mInfLightSelectionProbability(0)
	, mEmissiveSurfaceArea(0)
	, mEmissiveSurfacePower(0)
	, mEmissivePower(0)
{
	const float scene_area = 2 * PR_PI * scene->boundingSphere().radius(); /*Approximative*/ //scene->boundingSphere().surfaceArea();

	const auto& entities  = scene->database()->Entities->getAll();
	const auto& emissions = scene->database()->Emissions->getAll();
	const auto& inflights = scene->database()->InfiniteLights->getAll();

	// Get maximum amount of lights available
	size_t light_count = inflights.size();
	for (const auto& e : entities) {
		if (e->hasEmission())
			++light_count;
	}

	if (light_count == 0)
		return;

	const SpectralBlob test_wvl(550, 520, 460, 620);

	// Setup intensities
	std::vector<float> intensities;
	intensities.reserve(light_count);
	mLights.reserve(light_count);

	// Add area lights
	for (const auto& e : entities) {
		if (!e->hasEmission())
			continue;

		const uint32 ems_id = e->emissionID();
		if (ems_id >= emissions.size())
			continue;

		const IEmission* emission = emissions[ems_id].get();

		const float area	  = e->worldSurfaceArea();
		const float intensity = area * emission->power(test_wvl).mean();

		mEmissiveSurfaceArea += area;
		mEmissiveSurfacePower += intensity;

		PR_LOG(L_DEBUG) << "(Area) Light '" << e->name() << "' Area " << area << "m2 Intensity " << intensity << "W" << std::endl;

		intensities.push_back(intensity);
	}

	// Add infinite lights (approx) intensities
	mEmissivePower = mEmissiveSurfacePower;
	for (const auto& infL : inflights) {
		const float intensity = scene_area * infL->power(test_wvl).mean();
		mEmissivePower += intensity;

		PR_LOG(L_DEBUG) << "(Inf) Light '" << infL->name() << "' Area " << scene_area << "m2 Intensity " << intensity << "W" << std::endl;

		intensities.push_back(intensity);
	}

	// Generate distribution
	float full_approx_intensity = 0;
	mSelector					= std::make_unique<Distribution1D>(intensities.size());
	mSelector->generate([&](size_t i) { return intensities[i]; }, &full_approx_intensity);

	// Normalize intensities
	if (full_approx_intensity <= PR_EPSILON) {
		PR_LOG(L_WARNING) << "Lights are available but have no power" << std::endl;
	} else {
		// Normalize
		float invI = 1 / full_approx_intensity;
		for (float& f : intensities)
			f *= invI;
	}

	PR_ASSERT(full_approx_intensity == mEmissivePower, "Should be equal");

	// Finally add lights
	mLights.reserve(light_count);

	// Add area lights
	size_t k = 0;
	for (const auto& e : entities) {
		if (!e->hasEmission())
			continue;

		uint32 ems_id = e->emissionID();
		if (ems_id >= emissions.size())
			continue;

		IEmission* emission = emissions[ems_id].get();
		mLights.emplace_back(std::make_unique<Light>(Light::makeAreaLight(mLights.size(), e.get(), emission, intensities[k], this)));
		mLightEntityMap[e.get()] = k;
		++k;
	}

	// Add infinite lights (approx)
	for (const auto& infL : inflights) {
		mLights.emplace_back(std::make_unique<Light>(Light::makeInfLight(mLights.size(), infL.get(), intensities[k], this)));
		mInfLights.emplace_back(mLights.back().get());
		++k;
	}

	mInfLightSelectionProbability = (mEmissivePower - mEmissiveSurfacePower) / mEmissivePower;
}

} // namespace PR
