#include "renderer/RenderSettings.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_settings(py::module& m)
{
	py::class_<RenderSettings>(m, "RenderSettings")
		.def_readwrite("timeMappingMode", &RenderSettings::timeMappingMode)
		.def_readwrite("timeScale", &RenderSettings::timeScale)
		.def_readwrite("filmWidth", &RenderSettings::filmWidth)
		.def_readwrite("filmHeight", &RenderSettings::filmHeight)
		.def_readwrite("cropMaxX", &RenderSettings::cropMaxX)
		.def_readwrite("cropMinX", &RenderSettings::cropMinX)
		.def_readwrite("cropMaxY", &RenderSettings::cropMaxY)
		.def_readwrite("cropMinY", &RenderSettings::cropMinY)
		.def_readwrite("tileMode", &RenderSettings::tileMode)
		.def_readwrite("maxParallelRays", &RenderSettings::maxParallelRays)
		.def_readwrite("progressive", &RenderSettings::progressive)
		.def_readwrite("seed", &RenderSettings::seed)
		.def_readwrite("sortHits", &RenderSettings::sortHits)
		.def_readwrite("spectralHero", &RenderSettings::spectralHero)
		.def_readwrite("spectralMono", &RenderSettings::spectralMono)
		.def_readwrite("spectralStart", &RenderSettings::spectralStart)
		.def_readwrite("spectralEnd", &RenderSettings::spectralEnd)
		.def_readwrite("useAdaptiveTiling", &RenderSettings::useAdaptiveTiling);

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