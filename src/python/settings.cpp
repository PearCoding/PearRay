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
		.def_readwrite("tileMode", &RenderSettings::tileMode);

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