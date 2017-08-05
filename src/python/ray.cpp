#include "ray/Ray.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_ray(py::module& m)
{
	py::class_<Ray>(m, "Ray")
		.def(py::init<const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, uint32, float, uint8, uint16>(),
			py::arg("pixel"), py::arg("pos"), py::arg("dir"), py::arg("depth") = 0, py::arg("time") = 0, py::arg("wavelength") = 0, py::arg("flags") = 0)
		.def_property("origin", &Ray::origin, &Ray::setOrigin)
		.def_property("direction", &Ray::direction, &Ray::setDirection)
		.def_property("xorigin", &Ray::xorigin, &Ray::setXOrigin)
		.def_property("xdirection", &Ray::xdirection, &Ray::setXDirection)
		.def_property("yorigin", &Ray::yorigin, &Ray::setYOrigin)
		.def_property("ydirection", &Ray::ydirection, &Ray::setYDirection)
		.def_property("pixel", &Ray::pixel, &Ray::setPixel)
		.def_property("depth", &Ray::depth, &Ray::setDepth)
		.def_property("time", &Ray::time, &Ray::setTime)
		.def_property("wavelength", &Ray::wavelength, &Ray::setWavelength)
		.def_property("flags", &Ray::flags, &Ray::setFlags)
		.def("next", &Ray::next)
		.def_static("safe", &Ray::safe,
			py::arg("pixel"), py::arg("pos"), py::arg("dir"), py::arg("depth") = 0, py::arg("time") = 0, py::arg("wavelength") = 0, py::arg("flags") = 0);

	py::enum_<RayFlags>(m, "RayFlags", py::arithmetic())
		.value("LIGHT", RF_Light)
		.value("DEBUG", RF_Debug);
}
}