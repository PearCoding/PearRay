#include <thread>

#include "LogListener.h"
#include "Logger.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class LogListenerWrap : public LogListener {
public:
	void startEntry(LogLevel level) override
	{
		PYBIND11_OVERLOAD_PURE(void, LogListener, startEntry, level);
	}

	void writeEntry(int c) override
	{
		PYBIND11_OVERLOAD_PURE(void, LogListener, writeEntry, c);
	}
};

PR_NO_SANITIZE_ADDRESS
void setup_logger(py::module& m)
{
	py::class_<LogListener, LogListenerWrap>(m, "LogListener")
		.def("startEntry", &LogListener::startEntry)
		.def("writeEntry", &LogListener::writeEntry);

	py::class_<Logger>(m, "Logger")
		.def_property_readonly_static("instance", [](const py::object&) -> Logger& { return Logger::instance(); }, py::return_value_policy::reference)
		.def_property("verbosity", &Logger::verbosity, &Logger::setVerbosity)
		.def_property("quiet", &Logger::isQuiet, &Logger::setQuiet)
		.def("addListener", &Logger::addListener, py::keep_alive<1, 2>())
		.def("removeListener", &Logger::removeListener); // TODO: Remove assignment?

	py::enum_<LogLevel>(m, "LogLevel")
		.value("DEBUG", L_DEBUG)
		.value("INFO", L_INFO)
		.value("WARNING", L_WARNING)
		.value("ERROR", L_ERROR)
		.value("FATAL", L_FATAL);
}
}