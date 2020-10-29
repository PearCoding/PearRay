#include "LightSampler.h"
#include "Logger.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "scene/Scene.h"

namespace PR {

LightSampler::LightSampler(Scene* scene)
{
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

	// Setup buffers
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
		mLights.push_back(Light(e.get(), emission));

		const float intensity = e->worldSurfaceArea() * emission->power();
		intensities.push_back(intensity);
	}

	// Add infinite lights (approx)
	const float scene_radius = scene->boundingBox().diameter() / 2;
	const float scene_area	 = 4 * PR_PI * scene_radius * scene_radius;
	for (const auto& infL : inflights) {
		mLights.push_back(Light(infL.get()));
		const float intensity = scene_area * infL->power();
		intensities.push_back(intensity);
	}

	// Generate distribution
	float full_approx_intensity = 0;
	mSelector					= std::make_unique<Distribution1D>(intensities.size());
	mSelector->generate([&](size_t i) { return intensities[i]; }, &full_approx_intensity);

	PR_LOG(L_INFO) << "Approximative full light intensity within scene = " << full_approx_intensity << std::endl;
}

} // namespace PR
