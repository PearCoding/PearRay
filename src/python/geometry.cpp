#include "geometry/BoundingBox.h"
#include "geometry/Face.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "ray/Ray.h"
#include <boost/python.hpp>

#include "npmath.h"

using namespace PR;
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

namespace PRPY {
class BoundingBoxWrap : public BoundingBox, public bpy::wrapper<BoundingBox> {
public:
	BoundingBoxWrap()
		: BoundingBox()
	{
	}
	BoundingBoxWrap(const np::ndarray& upperbound, const np::ndarray& lowerbound)
		: BoundingBox(vec3FromPython(upperbound), vec3FromPython(lowerbound))
	{
	}
	BoundingBoxWrap(float width, float height, float depth)
		: BoundingBox(width, height, depth)
	{
	}

	BoundingBoxWrap(const boost::reference_wrapper<const BoundingBox>::type& other)
		: BoundingBox(other)
	{
	}

	// Better names?
	inline bpy::tuple intersects1_Py(const Ray& ray) const
	{
		float t;
		bool b = intersects(ray, t);
		return bpy::make_tuple(b, t);
	}

	inline bpy::tuple intersects2_Py(const Ray& ray) const
	{
		float t;
		Eigen::Vector3f collisionPoint;
		bool b = intersects(ray, collisionPoint, t);
		return bpy::make_tuple(b, vec3ToPython(collisionPoint), t);
	}

	inline bpy::tuple intersects3_Py(const Ray& ray) const
	{
		float t;
		Eigen::Vector3f collisionPoint;
		BoundingBox::FaceSide side;
		bool b = intersects(ray, collisionPoint, t, side);
		return bpy::make_tuple(b, vec3ToPython(collisionPoint), t, side);
	}

	void combine_Py(const np::ndarray& point)
	{
		combine(vec3FromPython(point));
	}

	BoundingBox combined_Py(const np::ndarray& point) const
	{
		return combined(vec3FromPython(point));
	}

	BoundingBox shifted_Py(const np::ndarray& point) const
	{
		return shifted(vec3FromPython(point));
	}

	PRPY_WRAP_GET_VEC3(upperBound)
	PRPY_WRAP_SET_VEC3(setUpperBound)
	PRPY_WRAP_GET_VEC3(lowerBound)
	PRPY_WRAP_SET_VEC3(setLowerBound)
	PRPY_WRAP_GET_VEC3(center)
};

class PlaneWrap : public Plane, public bpy::wrapper<Plane> {
public:
	PlaneWrap()
		: Plane()
	{
	}
	PlaneWrap(const np::ndarray& pos, const np::ndarray& xAxis, const np::ndarray& yAxis)
		: Plane(vec3FromPython(pos), vec3FromPython(xAxis), vec3FromPython(yAxis))
	{
	}
	PlaneWrap(float width, float height)
		: Plane(width, height)
	{
	}

	PlaneWrap(const boost::reference_wrapper<const Plane>::type& other)
		: Plane(other)
	{
	}

	inline bpy::tuple intersects_Py(const Ray& ray) const
	{
		Eigen::Vector3f pos;
		float t, u, v;
		bool b = intersects(ray, pos, t, u, v);
		return bpy::make_tuple(b, vec3ToPython(pos), t, u, v);
	}

	inline bpy::tuple project_Py(const np::ndarray& p) const
	{
		float u, v;
		project(vec3FromPython(p), u, v);
		return bpy::make_tuple(u, v);
	}

	bool contains_Py(const np::ndarray& point) const
	{
		return contains(vec3FromPython(point));
	}

	PRPY_WRAP_GET_VEC3(position)
	PRPY_WRAP_SET_VEC3(setPosition)
	PRPY_WRAP_GET_VEC3(xAxis)
	PRPY_WRAP_SET_VEC3(setXAxis)
	PRPY_WRAP_GET_VEC3(yAxis)
	PRPY_WRAP_SET_VEC3(setYAxis)
	PRPY_WRAP_GET_VEC3(normal)
	PRPY_WRAP_GET_VEC3(center)
};

class SphereWrap : public Sphere, public bpy::wrapper<Sphere> {
public:
	SphereWrap()
		: Sphere()
	{
	}
	SphereWrap(const np::ndarray& pos, float radius)
		: Sphere(vec3FromPython(pos), radius)
	{
	}

	SphereWrap(const boost::reference_wrapper<const Sphere>::type& other)
		: Sphere(other)
	{
	}

	inline bpy::tuple intersects_Py(const Ray& ray) const
	{
		Eigen::Vector3f pos;
		float t;
		bool b = intersects(ray, pos, t);
		return bpy::make_tuple(b, vec3ToPython(pos), t);
	}

	bool contains_Py(const np::ndarray& point) const
	{
		return contains(vec3FromPython(point));
	}

	void combine_Py(const np::ndarray& point)
	{
		combine(vec3FromPython(point));
	}

	Sphere combined_Py(const np::ndarray& point) const
	{
		return combined(vec3FromPython(point));
	}

	PRPY_WRAP_GET_VEC3(position)
	PRPY_WRAP_SET_VEC3(setPosition)
};

void setup_geometry()
{
	{
		bpy::scope scope = bpy::class_<BoundingBoxWrap>("BoundingBox")
							   .def(bpy::init<const np::ndarray&, const np::ndarray&>())
							   .def(bpy::init<float, float, float>())
							   .add_property("upperBound", &BoundingBoxWrap::upperBound_Py, &BoundingBoxWrap::setUpperBound_Py)
							   .add_property("lowerBound", &BoundingBoxWrap::lowerBound_Py, &BoundingBoxWrap::setLowerBound_Py)
							   .add_property("center", &BoundingBoxWrap::center_Py)
							   .add_property("width", &BoundingBox::width)
							   .add_property("height", &BoundingBox::height)
							   .add_property("depth", &BoundingBox::depth)
							   .add_property("outerSphere", &BoundingBox::outerSphere)
							   .add_property("innerSphere", &BoundingBox::innerSphere)
							   .add_property("volume", &BoundingBox::volume)
							   .add_property("surfaceArea", &BoundingBox::surfaceArea)
							   .def("isValid", &BoundingBox::isValid)
							   .def("isPlanar", &BoundingBox::isPlanar)
							   .def("contains", &BoundingBox::contains)
							   .def("intersects1", &BoundingBoxWrap::intersects1_Py)
							   .def("intersects2", &BoundingBoxWrap::intersects2_Py)
							   .def("intersects3", &BoundingBoxWrap::intersects3_Py)
							   .def("combine", &BoundingBoxWrap::combine_Py)
							   .def("combine", (void (BoundingBox::*)(const BoundingBox&)) & BoundingBox::combine)
							   .def("shift", &BoundingBox::shift)
							   .def("combined", &BoundingBoxWrap::combined_Py)
							   .def("combined", (BoundingBox(BoundingBox::*)(const BoundingBox&) const) & BoundingBox::combined)
							   .def("shifted", &BoundingBoxWrap::shifted_Py)
							   .def("getFace", &BoundingBox::getFace);

		bpy::enum_<BoundingBox::FaceSide>("FaceSide")
			.value("LEFT", BoundingBox::FS_Left)
			.value("RIGHT", BoundingBox::FS_Right)
			.value("TOP", BoundingBox::FS_Top)
			.value("BOTTOM", BoundingBox::FS_Bottom)
			.value("FRONT", BoundingBox::FS_Front)
			.value("BACK", BoundingBox::FS_Back);
	} // End of scope

	bpy::class_<PlaneWrap>("Plane")
		.def(bpy::init<const np::ndarray&, const np::ndarray&, const np::ndarray&>())
		.def(bpy::init<float, float>())
		.add_property("position", &PlaneWrap::position_Py, &PlaneWrap::setPosition_Py)
		.add_property("xAxis", &PlaneWrap::xAxis_Py, &PlaneWrap::setXAxis_Py)
		.add_property("yAxis", &PlaneWrap::yAxis_Py, &PlaneWrap::setYAxis_Py)
		.add_property("center", &PlaneWrap::center)
		.add_property("normal", &PlaneWrap::normal)
		.add_property("surfaceArea", &Plane::surfaceArea)
		.def("isValid", &Plane::isValid)
		.def("contains", &PlaneWrap::contains_Py)
		.def("intersects", &PlaneWrap::intersects_Py)
		.def("project", &PlaneWrap::project_Py)
		.add_property("boundingBox", &Plane::toBoundingBox)
		.add_property("localBoundingBox", &Plane::toLocalBoundingBox);

	bpy::class_<SphereWrap>("Sphere")
		.def(bpy::init<const np::ndarray&, float>())
		.add_property("position", &SphereWrap::position_Py, &SphereWrap::setPosition_Py)
		.add_property("radius", &Sphere::radius, &Sphere::setRadius)
		.add_property("volume", &Sphere::volume)
		.add_property("surfaceArea", &Sphere::surfaceArea)
		.def("isValid", &Sphere::isValid)
		.def("contains", &SphereWrap::contains_Py)
		.def("intersects", &SphereWrap::intersects_Py)
		.def("combine", &SphereWrap::combine_Py)
		.def("combined", &SphereWrap::combined_Py)
		.def("combine", (void (Sphere::*)(const Sphere&))&Sphere::combine)
		.def("combined", (Sphere (Sphere::*)(const Sphere&)const)&Sphere::combined);
}
}