#include "renderer/RenderSettings.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_settings(py::module& m)
{
	py::class_<RenderSettings>(m, "RenderSettings")
		.def_property_readonly("aaSampler", &RenderSettings::aaSampler)
		.def_property_readonly("aaSampleCount", &RenderSettings::aaSampleCount)
		.def_property_readonly("lensSampler", &RenderSettings::lensSampler)
		.def_property_readonly("lensSampleCount", &RenderSettings::lensSampleCount)
		.def_property_readonly("timeSampler", &RenderSettings::timeSampler)
		.def_property_readonly("timeSampleCount", &RenderSettings::timeSampleCount)
		.def_property_readonly("timeMappingMode", &RenderSettings::timeMappingMode)
		.def_property_readonly("timeScale", &RenderSettings::timeScale)
		.def_property_readonly("spectralSampler", &RenderSettings::spectralSampler)
		.def_property_readonly("spectralSampleCount", &RenderSettings::spectralSampleCount)
		.def_property_readonly("samplesPerPixel", &RenderSettings::samplesPerPixel)
		.def_property_readonly("maxRayDepth", &RenderSettings::maxRayDepth)
		.def_property_readonly("cropMaxX", &RenderSettings::cropMaxX)
		.def_property_readonly("cropMinX", &RenderSettings::cropMinX)
		.def_property_readonly("cropMaxY", &RenderSettings::cropMaxY)
		.def_property_readonly("cropMinY", &RenderSettings::cropMinY)
		.def_property_readonly("tileMode", &RenderSettings::tileMode);

	py::enum_<SamplerMode>(m, "SamplerMode")
		.value("RANDOM", SM_RANDOM)
		.value("UNIFORM", SM_UNIFORM)
		.value("JITTER", SM_JITTER)
		.value("MULTI_JITTER", SM_MULTI_JITTER)
		.value("HALTON_QMC", SM_HALTON_QMC);

	py::enum_<DebugMode>(m, "DebugMode")
		.value("DEPTH", DM_DEPTH)
		.value("NORMAL_BOTH", DM_NORMAL_BOTH)
		.value("NORMAL_POSITIVE", DM_NORMAL_POSITIVE)
		.value("NORMAL_NEGATIVE", DM_NORMAL_NEGATIVE)
		.value("NORMAL_SPHERICAL", DM_NORMAL_SPHERICAL)
		.value("TANGENT_BOTH", DM_TANGENT_BOTH)
		.value("TANGENT_POSITIVE", DM_TANGENT_POSITIVE)
		.value("TANGENT_NEGATIVE", DM_TANGENT_NEGATIVE)
		.value("TANGENT_SPHERICAL", DM_TANGENT_SPHERICAL)
		.value("BINORMAL_BOTH", DM_BINORMAL_BOTH)
		.value("BINORMAL_POSITIVE", DM_BINORMAL_POSITIVE)
		.value("BINORMAL_NEGATIVE", DM_BINORMAL_NEGATIVE)
		.value("BINORMAL_SPHERICAL", DM_BINORMAL_SPHERICAL)
		.value("UVW", DM_UVW)
		.value("PDF", DM_PDF)
		.value("EMISSION", DM_EMISSION)
		.value("VALIDITY", DM_VALIDITY)
		.value("FLAG_INSIDE", DM_FLAG_INSIDE)
		.value("CONTAINER_ID", DM_CONTAINER_ID)
		.value("BOUNDING_BOX", DM_BOUNDING_BOX);

	py::enum_<IntegratorMode>(m, "IntegratorMode")
		.value("DIRECT", IM_DIRECT)
		.value("BIDIRECT", IM_BIDIRECT)
		.value("PPM", IM_PPM)
		.value("AO", IM_AO)
		.value("VISUALIZER", IM_VISUALIZER);

	py::enum_<TileMode>(m, "TileMode")
		.value("LINEAR", TM_LINEAR)
		.value("TILE", TM_TILE)
		.value("SPIRAL", TM_SPIRAL);

	py::enum_<TimeMappingMode>(m, "TimeMappingMode")
		.value("CENTER", TMM_CENTER)
		.value("LEFT", TMM_LEFT)
		.value("RIGHT", TMM_RIGHT);

	py::enum_<PPMGatheringMode>(m, "PPMGatheringMode")
		.value("SPHERE", PGM_SPHERE)
		.value("DOME", PGM_DOME);
}
}