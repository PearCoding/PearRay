#include "ray/Ray.h"
#include <boost/python.hpp>

#include "npmath.h"

using namespace PR;
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

namespace PRPY {
class RayWrap : public Ray, public bpy::wrapper<Ray> {
public:
	RayWrap()
		: Ray()
	{
	}

	RayWrap(const np::ndarray& p, const np::ndarray& o, const np::ndarray& d,
			uint32 depth = 0, float time = 0, uint8 wavelength = 0, uint16 flags = 0)
		: Ray(ivec2FromPython(p), vec3FromPython(o), vec3FromPython(d), depth, time, wavelength, flags)
	{
	}

	RayWrap(const boost::reference_wrapper<const Ray>::type& other)
		: Ray(other)
	{
	}

	PRPY_WRAP_GET_IVEC2(pixel)
	PRPY_WRAP_SET_IVEC2(setPixel)

	PRPY_WRAP_GET_VEC3(origin)
	PRPY_WRAP_SET_VEC3(setOrigin)

	PRPY_WRAP_GET_VEC3(direction)
	PRPY_WRAP_SET_VEC3(setDirection)

	PRPY_WRAP_GET_VEC3(xorigin)
	PRPY_WRAP_SET_VEC3(setXOrigin)

	PRPY_WRAP_GET_VEC3(xdirection)
	PRPY_WRAP_SET_VEC3(setXDirection)

	PRPY_WRAP_GET_VEC3(yorigin)
	PRPY_WRAP_SET_VEC3(setYOrigin)

	PRPY_WRAP_GET_VEC3(ydirection)
	PRPY_WRAP_SET_VEC3(setYDirection)

	float time_Py() const { return (float)time(); }
	void setTime_Py(float t) { setTime(t); }

	Ray next_Py(const np::ndarray& pos, const np::ndarray& dir) const
	{
		return next(vec3FromPython(pos), vec3FromPython(dir));
	}

	static Ray safe_Py(const np::ndarray& pixel, const np::ndarray& pos, const np::ndarray& dir,
					   uint32 depth = 0, float time = 0, uint8 wavelength = 0, uint16 flags = 0)
	{
		return safe(ivec2FromPython(pixel), vec3FromPython(pos), vec3FromPython(dir),
					depth, time, wavelength, flags);
	}
};

void setup_ray()
{
	bpy::class_<RayWrap>("Ray")
		.def(bpy::init<const np::ndarray&, const np::ndarray&, const np::ndarray&, bpy::optional<uint32, float, uint8, uint16>>())
		.add_property("origin", &RayWrap::origin_Py, &RayWrap::setOrigin_Py)
		.add_property("direction", &RayWrap::direction_Py, &RayWrap::setDirection_Py)
		.add_property("xorigin", &RayWrap::xorigin_Py, &RayWrap::setXOrigin_Py)
		.add_property("xdirection", &RayWrap::xdirection_Py, &RayWrap::setXDirection_Py)
		.add_property("yorigin", &RayWrap::yorigin_Py, &RayWrap::setYOrigin_Py)
		.add_property("ydirection", &RayWrap::ydirection_Py, &RayWrap::setYDirection_Py)
		.add_property("pixel", &RayWrap::pixel, &RayWrap::setPixel)
		.add_property("depth", &Ray::depth, &Ray::setDepth)
		.add_property("time", &RayWrap::time_Py, &RayWrap::setTime_Py)
		.add_property("wavelength", &Ray::wavelength, &Ray::setWavelength)
		.add_property("flags", &Ray::flags, &Ray::setFlags)

		.def("next", &RayWrap::next_Py)
		.add_property("safe", bpy::make_function(
								  (Ray(*)(const np::ndarray&, const np::ndarray&, const np::ndarray&, uint32, float, uint8, uint16)) & RayWrap::safe_Py))
		.add_property("safe", bpy::make_function(
								  (Ray(*)(const np::ndarray&, const np::ndarray&, const np::ndarray&, uint32, float, uint8)) & RayWrap::safe_Py))
		.add_property("safe", bpy::make_function(
								  (Ray(*)(const np::ndarray&, const np::ndarray&, const np::ndarray&, uint32, float)) & RayWrap::safe_Py))
		.add_property("safe", bpy::make_function(
								  (Ray(*)(const np::ndarray&, const np::ndarray&, const np::ndarray&, uint32)) & RayWrap::safe_Py))
		.add_property("safe", bpy::make_function(
								  (Ray(*)(const np::ndarray&, const np::ndarray&, const np::ndarray&)) & RayWrap::safe_Py));

	bpy::enum_<RayFlags>("RayFlags")
		.value("LIGHT", RF_Light)
		.value("DEBUG", RF_Debug);
}
}