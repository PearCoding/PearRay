#include "camera/Camera.h"
#include "camera/StandardCamera.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"

#include <boost/python.hpp>

using namespace PR;
namespace bpy = boost::python;
namespace PRPY {
class CameraWrap : public Camera, public bpy::wrapper<Camera> {
public:
	CameraWrap(uint32 id, const std::string& name)
		: Camera(id, name)
	{
	}

	inline Ray constructRay(RenderContext* context, const CameraSample& sample) const override
	{
		return this->get_override("constructRay")(context, sample);
	}
};

void setup_camera()
{
	bpy::class_<CameraSample, std::shared_ptr<CameraSample>>("CameraSample")
		/* TODO */
		;

	bpy::class_<CameraWrap, std::shared_ptr<CameraWrap>, bpy::bases<Entity>, boost::noncopyable>("Camera", bpy::init<uint32, std::string>())
		.def("constructRay", bpy::pure_virtual(&Camera::constructRay));

	bpy::class_<StandardCamera, std::shared_ptr<StandardCamera>, bpy::bases<Camera>, boost::noncopyable>("StandardCamera", bpy::init<uint32, std::string>())
		.add_property("width", &StandardCamera::width, &StandardCamera::setWidth)
		.add_property("height", &StandardCamera::height, &StandardCamera::setHeight)
		.add_property("localDirection", &StandardCamera::localDirection, &StandardCamera::setLocalDirection)
		.add_property("localRight", &StandardCamera::localRight, &StandardCamera::setLocalRight)
		.add_property("localUp", &StandardCamera::localUp, &StandardCamera::setLocalUp)
		.add_property("fstop", &StandardCamera::fstop, &StandardCamera::setFStop)
		.add_property("apertureRadius", &StandardCamera::apertureRadius, &StandardCamera::setApertureRadius)
		.def("setWithAngle", &StandardCamera::setWithAngle)
		.def("setWithSize", &StandardCamera::setWithSize);

	bpy::register_ptr_to_python<std::shared_ptr<Camera>>();
	bpy::implicitly_convertible<std::shared_ptr<Camera>, std::shared_ptr<Entity>>();
	bpy::implicitly_convertible<std::shared_ptr<StandardCamera>, std::shared_ptr<Camera>>();
}
}