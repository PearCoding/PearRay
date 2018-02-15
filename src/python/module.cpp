#include "pypearray.h"
#include <sstream>

std::tuple<int,int> version()
{
	return std::make_tuple(PR_VERSION_MAJOR, PR_VERSION_MINOR);
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
void setup_renderentities(py::module& m);
void setup_light(py::module& m);
void setup_sampler(py::module& m);
void setup_status(py::module& m);
void setup_renderer(py::module& m);
void setup_tonemapper(py::module& m);
void setup_environment(py::module& m);
}

//----------
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
	PRPY::setup_renderentities(m);
	PRPY::setup_light(m);
	PRPY::setup_scene(m);
	PRPY::setup_settings(m);
	PRPY::setup_output(m);
	PRPY::setup_status(m);
	PRPY::setup_renderer(m);
	PRPY::setup_tonemapper(m);
	PRPY::setup_environment(m);
}