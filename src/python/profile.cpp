#include "Profiler.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_profiler(py::module& m)
{
	auto sm = m.def_submodule("Profiler");
	sm.def("start",
		   [](int samples) {
#ifdef PR_WITH_PROFILER
			   Profiler::start(samples);
#else
			   (void)samples;
#endif
		   },
		   py::arg("samplesPerSecond") = 10);

	sm.def("stop",
		   []() {
#ifdef PR_WITH_PROFILER
			   Profiler::stop();
#endif
		   });

	sm.def("dumpToFile",
		   [](const std::wstring& filename) {
#ifdef PR_WITH_PROFILER
			   Profiler::dumpToFile(filename);
#else
			   (void)filename;
#endif
		   });
}
} // namespace PRPY