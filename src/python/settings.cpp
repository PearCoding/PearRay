#include "renderer/PPMSettings.h"
#include "renderer/RenderSettings.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_settings(py::module& m)
{
	py::class_<RenderSettings>(m, "RenderSettings")
		.def_property("debugMode", &RenderSettings::debugMode, &RenderSettings::setDebugMode)
		.def_property("integratorMode", &RenderSettings::integratorMode, &RenderSettings::setIntegratorMode)
		.def_property("aaSampler", &RenderSettings::aaSampler, &RenderSettings::setAASampler)
		.def_property("maxAASampleCount", &RenderSettings::maxAASampleCount, &RenderSettings::setMaxAASampleCount)
		.def_property("lensSampler", &RenderSettings::lensSampler, &RenderSettings::setLensSampler)
		.def_property("maxLensSampleCount", &RenderSettings::maxLensSampleCount, &RenderSettings::setMaxLensSampleCount)
		.def_property("timeSampler", &RenderSettings::timeSampler, &RenderSettings::setTimeSampler)
		.def_property("maxTimeSampleCount", &RenderSettings::maxTimeSampleCount, &RenderSettings::setMaxTimeSampleCount)
		.def_property("timeMappingMode", &RenderSettings::timeMappingMode, &RenderSettings::setTimeMappingMode)
		.def_property("timeScale", &RenderSettings::timeScale, &RenderSettings::setTimeScale)
		.def_property("spectralSampler", &RenderSettings::spectralSampler, &RenderSettings::setSpectralSampler)
		.def_property("maxSpectralSampleCount", &RenderSettings::maxSpectralSampleCount, &RenderSettings::setMaxSpectralSampleCount)
		.def_property_readonly("maxCameraSampleCount", &RenderSettings::maxCameraSampleCount)
		.def_property("maxDiffuseBounces", &RenderSettings::maxDiffuseBounces, &RenderSettings::setMaxDiffuseBounces)
		.def_property("maxRayDepth", &RenderSettings::maxRayDepth, &RenderSettings::setMaxRayDepth)
		.def_property("cropMaxX", &RenderSettings::cropMaxX, &RenderSettings::setCropMaxX)
		.def_property("cropMinX", &RenderSettings::cropMinX, &RenderSettings::setCropMinX)
		.def_property("cropMaxY", &RenderSettings::cropMaxY, &RenderSettings::setCropMaxY)
		.def_property("cropMinY", &RenderSettings::cropMinY, &RenderSettings::setCropMinY)
		.def_property("maxLightSamples", &RenderSettings::maxLightSamples, &RenderSettings::setMaxLightSamples)
		.def_property_readonly("ppm", (PPMSettings & (RenderSettings::*)()) & RenderSettings::ppm, py::return_value_policy::reference)
		.def_property("tileMode", &RenderSettings::tileMode, &RenderSettings::setTileMode);

	py::class_<PPMSettings>(m, "PPMSettings")
		.def_property("maxPhotonsPerPass", &PPMSettings::maxPhotonsPerPass, &PPMSettings::setMaxPhotonsPerPass)
		.def_property("maxPassCount", &PPMSettings::maxPassCount, &PPMSettings::setMaxPassCount)
		.def_property("maxGatherRadius", &PPMSettings::maxGatherRadius, &PPMSettings::setMaxGatherRadius)
		.def_property("maxGatherCount", &PPMSettings::maxGatherCount, &PPMSettings::setMaxGatherCount)
		.def_property("gatheringMode", &PPMSettings::gatheringMode, &PPMSettings::setGatheringMode)
		.def_property("squeezeWeight", &PPMSettings::squeezeWeight, &PPMSettings::setSqueezeWeight)
		.def_property("contractRatio", &PPMSettings::contractRatio, &PPMSettings::setContractRatio);

	py::enum_<SamplerMode>(m, "SamplerMode")
		.value("RANDOM", SM_Random)
		.value("UNIFORM", SM_Uniform)
		.value("JITTER", SM_Jitter)
		.value("MULTIJITTER", SM_MultiJitter)
		.value("HALTONQMC", SM_HaltonQMC);

	py::enum_<DebugMode>(m, "DebugMode")
		.value("NONE", DM_None)
		.value("DEPTH", DM_Depth)
		.value("NORMAL_BOTH", DM_Normal_Both)
		.value("NORMAL_POSITIVE", DM_Normal_Positive)
		.value("NORMAL_NEGATIVE", DM_Normal_Negative)
		.value("NORMAL_SPHERICAL", DM_Normal_Spherical)
		.value("TANGENT_BOTH", DM_Tangent_Both)
		.value("TANGENT_POSITIVE", DM_Tangent_Positive)
		.value("TANGENT_NEGATIVE", DM_Tangent_Negative)
		.value("TANGENT_SPHERICAL", DM_Tangent_Spherical)
		.value("BINORMAL_BOTH", DM_Binormal_Both)
		.value("BINORMAL_POSITIVE", DM_Binormal_Positive)
		.value("BINORMAL_NEGATIVE", DM_Binormal_Negative)
		.value("BINORMAL_SPHERICAL", DM_Binormal_Spherical)
		.value("UVW", DM_UVW)
		.value("PDF", DM_PDF)
		.value("EMISSION", DM_Emission)
		.value("VALIDITY", DM_Validity)
		.value("FLAG_INSIDE", DM_Flag_Inside)
		.value("CONTAINER_ID", DM_Container_ID);

	py::enum_<IntegratorMode>(m, "IntegratorMode")
		.value("DIRECT", IM_Direct)
		.value("BIDIRECT", IM_BiDirect)
		.value("PPM", IM_PPM);

	py::enum_<TileMode>(m, "TileMode")
		.value("LINEAR", TM_Linear)
		.value("TILE", TM_Tile)
		.value("SPIRAL", TM_Spiral);

	py::enum_<TimeMappingMode>(m, "TimeMappingMode")
		.value("CENTER", TMM_Center)
		.value("LEFT", TMM_Left)
		.value("RIGHT", TMM_Right);

	py::enum_<PPMGatheringMode>(m, "PPMGatheringMode")
		.value("SPHERE", PGM_Sphere)
		.value("DOME", PGM_Dome);
}
}