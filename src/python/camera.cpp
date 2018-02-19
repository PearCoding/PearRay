#include "camera/Camera.h"
#include "camera/StandardCamera.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_camera(py::module& m)
{
	py::class_<Camera, std::shared_ptr<Camera>>(m, "Camera");

	py::class_<StandardCamera, std::shared_ptr<StandardCamera>, Camera>(m, "StandardCamera")
		.def(py::init<uint32, std::string>())
		.def_property("width", &StandardCamera::width, &StandardCamera::setWidth)
		.def_property("height", &StandardCamera::height, &StandardCamera::setHeight)
		.def_property("localDirection", &StandardCamera::localDirection, &StandardCamera::setLocalDirection)
		.def_property("localRight", &StandardCamera::localRight, &StandardCamera::setLocalRight)
		.def_property("localUp", &StandardCamera::localUp, &StandardCamera::setLocalUp)
		.def_property("fstop", &StandardCamera::fstop, &StandardCamera::setFStop)
		.def_property("apertureRadius", &StandardCamera::apertureRadius, &StandardCamera::setApertureRadius)
		.def("setWithAngle", &StandardCamera::setWithAngle)
		.def("setWithSize", &StandardCamera::setWithSize);
}
}