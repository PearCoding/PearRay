#include "camera/Camera.h"
#include "camera/StandardCamera.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class CameraWrap : public Camera {
public:
	using Camera::Camera;

	inline Ray constructRay(RenderContext* context, const CameraSample& sample) const override
	{
		PYBIND11_OVERLOAD_PURE(
			Ray,			/* Return type */
			Camera,			/* Parent class */
			constructRay,   /* Name of function in C++ (must match Python name) */
			context, sample /* Argument(s) */
			);
	}
};

void setup_camera(py::module& m)
{
	py::class_<CameraSample, std::shared_ptr<CameraSample>>(m, "CameraSample")
		/* TODO */
		;

	py::class_<Camera, CameraWrap, std::shared_ptr<Camera>>(m, "Camera")
		.def(py::init<uint32, std::string>())
		.def("constructRay", &Camera::constructRay);

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