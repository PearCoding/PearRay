#include "renderer/RenderSettings.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_settings(py::module& m)
{
	py::class_<RenderSettings>(m, "RenderSettings")
		.def_readonly("aaSampler", &RenderSettings::aaSampler)
		.def_readonly("aaSampleCount", &RenderSettings::aaSampleCount)
		.def_readonly("lensSampler", &RenderSettings::lensSampler)
		.def_readonly("lensSampleCount", &RenderSettings::lensSampleCount)
		.def_readonly("timeSampler", &RenderSettings::timeSampler)
		.def_readonly("timeSampleCount", &RenderSettings::timeSampleCount)
		.def_readonly("timeMappingMode", &RenderSettings::timeMappingMode)
		.def_readonly("timeScale", &RenderSettings::timeScale)
		.def_property_readonly("samplesPerPixel", &RenderSettings::samplesPerPixel)
		.def_readonly("maxRayDepth", &RenderSettings::maxRayDepth)
		.def_readonly("filmWidth", &RenderSettings::filmWidth)
		.def_readonly("filmHeight", &RenderSettings::filmHeight)
		.def_readonly("cropMaxX", &RenderSettings::cropMaxX)
		.def_readonly("cropMinX", &RenderSettings::cropMinX)
		.def_readonly("cropMaxY", &RenderSettings::cropMaxY)
		.def_readonly("cropMinY", &RenderSettings::cropMinY)
		.def_readonly("tileMode", &RenderSettings::tileMode);

	py::enum_<TileMode>(m, "TileMode")
		.value("LINEAR", TM_LINEAR)
		.value("TILE", TM_TILE)
		.value("SPIRAL", TM_SPIRAL);

	py::enum_<TimeMappingMode>(m, "TimeMappingMode")
		.value("CENTER", TMM_CENTER)
		.value("LEFT", TMM_LEFT)
		.value("RIGHT", TMM_RIGHT);
}
}