#include "GlobalPropertyLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "DataLisp.h"

#include <fstream>
#include <sstream>

namespace PR {
RenderSettings GlobalPropertyLoader::loadGlobalProperties(const DL::DataGroup& group)
{
	RenderSettings settings;

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data dataD = group.at(i);

		if (dataD.type() == DL::Data::T_Group) {
			DL::DataGroup entry = dataD.getGroup();

			if (entry.id() == "property") {
				handleProperty(entry, settings);
			}
		}
	}

	return settings;
}

void GlobalPropertyLoader::handleProperty(const DL::DataGroup& group, RenderSettings& settings)
{
	/*if(entry.id() != "incremental")
			settings.setIncremental(handleBool(entry, settings.isIncremental()));
		else if(entry.id() != "debug")
			settings.setDebugMode(handleDebugMode(entry, settings.debugMode()));
		else if(entry.id() != "integrator")
			settings.setIntegratorMode(handleIntegratorMode(entry, settings.integratorMode()));

		else if(entry.id() != "aa_mode")
			settings.setAASampler(handleSamplerMode(entry, settings.aaSampler()));
		else if(entry.id() != "aa_max")
			settings.setMaxAASampleCount(handleInteger(entry, settings.maxAASampleCount()));
		else if(entry.id() != "lens_mode")
			settings.setLensSampler(handleSamplerMode(entry, settings.lensSampler()));
		else if(entry.id() != "lens_max")
			settings.setMaxLensSampleCount(handleInteger(entry, settings.maxLensSampleCount()));
		else if(entry.id() != "time_mode")
			settings.setTimeSampler(handleSamplerMode(entry, settings.timeSampler()));
		else if(entry.id() != "time_max")
			settings.setMaxTimeSampleCount(handleInteger(entry, settings.maxTimeSampleCount()));
		else if(entry.id() != "time_mapping")
			settings.setTimeMappingMode(handleTimeMappingMode(entry, settings.timeMappingMode()));
		else if(entry.id() != "time_scale")
			settings.setTimeScale(handleNumber(entry, settings.timeScale()));
		else if(entry.id() != "spectral_mode")
			settings.setSpectralSampler(handleSamplerMode(entry, settings.spectralSampler()));
		else if(entry.id() != "spectral_max")
			settings.setMaxSpectralSampleCount(handleInteger(entry, settings.maxSpectralSampleCount()));

		else if(entry.id() != "diffuse_bounces")
			settings.setMaxDiffuseBounces(handleInteger(entry, settings.maxDiffuseBounces()));
		else if(entry.id() != "ray_max")
			settings.setMaxRayDepth(handleInteger(entry, settings.maxRayDepth()));
		else if(entry.id() != "light_samples")
			settings.setMaxLightSamples(handleInteger(entry, settings.maxLightSamples()));
		*/
	// Tile Mode??
}
} // namespace PR
