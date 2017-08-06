#include <thread>

#include "LogListener.h"
#include "Logger.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class LogListenerWrap : public LogListener {
public:
	void newEntry(Level level, Module m, const std::string& str) override
	{
		PYBIND11_OVERLOAD_PURE(void, LogListener, newEntry, level, m, str);
	}
};

void setup_logger(py::module& m)
{
	py::class_<LogListener, LogListenerWrap>(m, "LogListener")
		.def("newEntry", &LogListener::newEntry);

	py::class_<Logger>(m, "Logger")
		.def_property_readonly_static("instance", [](const py::object&) -> Logger& { return Logger::instance(); }, py::return_value_policy::reference)
		.def_property("verbose", &Logger::isVerbose, &Logger::setVerbose)
		.def_property("quiet", &Logger::isQuiet, &Logger::setQuiet)
		.def("log", &Logger::log)
		.def("addListener", &Logger::addListener, py::keep_alive<1, 2>())
		.def("removeListener", &Logger::removeListener);// TODO: Remove assignment?

	py::enum_<Level>(m, "Level")
		.value("DEBUG", L_Debug)
		.value("INFO", L_Info)
		.value("WARNING", L_Warning)
		.value("ERROR", L_Error)
		.value("FATAL", L_Fatal);

	py::enum_<Module>(m, "Module")
		.value("INTERNAL", M_Internal)
		.value("TEST", M_Test)
		.value("CAMERA", M_Camera)
		.value("GPU", M_GPU)
		.value("ENTITY", M_Entity)
		.value("MATH", M_Math)
		.value("INTEGRATOR", M_Integrator)
		.value("MATERIAL", M_Material)
		.value("SYSTEM", M_System)
		.value("SCENE", M_Scene)
		.value("VOLUME", M_Volume)
		.value("NETWORK", M_Network)
		.value("LOADER", M_Loader)
		.value("SHADER", M_Shader)
		.value("MAIN", M_Main);
}
}