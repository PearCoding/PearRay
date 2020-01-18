#include "config/Build.h"
#include "pypearray.h"

#include <sstream>

std::tuple<int, int> version()
{
	auto version = PR::Build::getVersion();
	return std::make_tuple(version.Major, version.Minor);
}

namespace PRPY {
void setup_logger(py::module& m);
void setup_spectral(py::module& m);
void setup_entity(py::module& m);
void setup_shader(py::module& m);
void setup_math(py::module& m);
void setup_material(py::module& m);
void setup_settings(py::module& m);
void setup_output(py::module& m);
void setup_scene(py::module& m);
void setup_geometry(py::module& m);
void setup_camera(py::module& m);
void setup_infinitelight(py::module& m);
void setup_sampler(py::module& m);
void setup_status(py::module& m);
void setup_renderer(py::module& m);
void setup_tonemapper(py::module& m);
void setup_environment(py::module& m);
void setup_integrator(py::module& m);
void setup_profiler(py::module& m);
} // namespace PRPY

/* ATTENTION
 * Don't expose Type* getFoo() when smart pointers are involved!
 * See https://pybind11.readthedocs.io/en/stable/advanced/smart_ptrs.html
 */
//----------
PR_NO_SANITIZE_ADDRESS
PYBIND11_MODULE(pypearray, m)
{
	m.doc() = "PearRay Python API";

	m.def("version", &version, "Returns tuple representing the version of the API");

	PRPY::setup_math(m);
	PRPY::setup_logger(m);
	PRPY::setup_spectral(m);
	PRPY::setup_sampler(m);
	PRPY::setup_geometry(m);
	PRPY::setup_shader(m);
	PRPY::setup_material(m);
	PRPY::setup_entity(m);
	PRPY::setup_camera(m);
	PRPY::setup_infinitelight(m);
	PRPY::setup_scene(m);
	PRPY::setup_settings(m);
	PRPY::setup_output(m);
	PRPY::setup_status(m);
	PRPY::setup_renderer(m);
	PRPY::setup_tonemapper(m);
	PRPY::setup_integrator(m);
	PRPY::setup_environment(m);
	PRPY::setup_profiler(m);
}