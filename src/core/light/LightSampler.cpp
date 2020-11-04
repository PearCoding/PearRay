#include "LightSampler.h"
#include "Logger.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "scene/Scene.h"

namespace PR {

LightSampler::LightSampler(Scene* scene)
	: mEmissiveSurfaceArea(0)
	, mEmissiveSurfacePower(0)
	, mEmissivePower(0)
{
	const float scene_area = 2 * PR_PI * scene->boundingSphere().radius(); /*Approximative*/ //scene->boundingSphere().surfaceArea();

	const auto& entities  = scene->entities();
	const auto& emissions = scene->emissions();
	const auto& inflights = scene->infiniteLights();

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

		uint32 ems_id = e->emissionID();
		if (ems_id >= emissions.size())
			continue;

		IEmission* emission = emissions[ems_id].get();

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

	PR_LOG(L_INFO) << "Approximative full light intensity within scene = " << full_approx_intensity << std::endl;

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
		mLights.emplace_back(std::make_unique<Light>(Light::makeAreaLight(e.get(), emission, intensities[k])));
		mLightEntityMap[e.get()] = k;
		++k;
	}

	// Add infinite lights (approx)
	for (const auto& infL : inflights) {
		mLights.emplace_back(std::make_unique<Light>(Light::makeInfLight(infL.get(), intensities[k])));
		++k;
	}
}

} // namespace PR
