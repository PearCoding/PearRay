#include "geometry/BoundingBox.h"
#include "geometry/Face.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "ray/Ray.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_geometry(py::module& m)
{
	auto scope = py::class_<BoundingBox>(m, "BoundingBox");
	scope.def(py::init<const Vector3f&, const Vector3f&>())
		.def(py::init<float, float, float>())
		.def_property("upperBound", (const Vector3f& (BoundingBox::*)() const) & BoundingBox::upperBound, &BoundingBox::setUpperBound)
		.def_property("lowerBound", (const Vector3f& (BoundingBox::*)() const) & BoundingBox::lowerBound, &BoundingBox::setLowerBound)
		.def_property_readonly("center", &BoundingBox::center)
		.def_property_readonly("width", &BoundingBox::width)
		.def_property_readonly("height", &BoundingBox::height)
		.def_property_readonly("depth", &BoundingBox::depth)
		.def_property_readonly("outerSphere", &BoundingBox::outerSphere)
		.def_property_readonly("innerSphere", &BoundingBox::innerSphere)
		.def_property_readonly("volume", &BoundingBox::volume)
		.def_property_readonly("surfaceArea", &BoundingBox::surfaceArea)
		.def("isValid", &BoundingBox::isValid)
		.def("isPlanar", &BoundingBox::isPlanar)
		.def("contains", &BoundingBox::contains)
		.def("intersects", &BoundingBox::intersects)
		.def("intersectsSimple", &BoundingBox::intersectsSimple)
		.def("intersectsRange", &BoundingBox::intersectsRange)
		.def("getIntersectionSide", &BoundingBox::getIntersectionSide)
		.def("combine", (void (BoundingBox::*)(const Vector3f&)) & BoundingBox::combine)
		.def("combine", (void (BoundingBox::*)(const BoundingBox&)) & BoundingBox::combine)
		.def("shift", &BoundingBox::shift)
		.def("combined", (BoundingBox(BoundingBox::*)(const Vector3f&) const) & BoundingBox::combined)
		.def("combined", (BoundingBox(BoundingBox::*)(const BoundingBox&) const) & BoundingBox::combined)
		.def("shifted", &BoundingBox::shifted)
		.def("getFace", &BoundingBox::getFace);

	py::enum_<BoundingBox::FaceSide>(scope, "FaceSide")
		.value("LEFT", BoundingBox::FS_Left)
		.value("RIGHT", BoundingBox::FS_Right)
		.value("TOP", BoundingBox::FS_Top)
		.value("BOTTOM", BoundingBox::FS_Bottom)
		.value("FRONT", BoundingBox::FS_Front)
		.value("BACK", BoundingBox::FS_Back);

	py::class_<BoundingBox::Intersection>(scope, "Intersection")
		.def_readwrite("Successful", &BoundingBox::Intersection::Successful)
		.def_readwrite("Position", &BoundingBox::Intersection::Position)
		.def_readwrite("T", &BoundingBox::Intersection::T);

	py::class_<BoundingBox::IntersectionRange>(scope, "IntersectionRange")
		.def_readwrite("Successful", &BoundingBox::IntersectionRange::Successful)
		.def_readwrite("Entry", &BoundingBox::IntersectionRange::Entry)
		.def_readwrite("Exit", &BoundingBox::IntersectionRange::Exit);

	/////////////////////////////////////////////////////
	auto scope2 = py::class_<Plane>(m, "Plane");
	scope2.def(py::init<const Vector3f&, const Vector3f&, const Vector3f&>())
		.def(py::init<float, float>())
		.def_property("position", &Plane::position, &Plane::setPosition)
		.def_property("xAxis", &Plane::xAxis, &Plane::setXAxis)
		.def_property("yAxis", &Plane::yAxis, &Plane::setYAxis)
		.def_property_readonly("center", &Plane::center)
		.def_property_readonly("normal", &Plane::normal)
		.def_property_readonly("surfaceArea", &Plane::surfaceArea)
		.def("isValid", &Plane::isValid)
		.def("contains", &Plane::contains)
		.def("intersects", &Plane::intersects)
		.def("project", &Plane::project)
		.def_property_readonly("boundingBox", &Plane::toBoundingBox)
		.def_property_readonly("localBoundingBox", &Plane::toLocalBoundingBox);

	py::class_<Plane::Intersection>(scope2, "Intersection")
		.def_readwrite("Successful", &Plane::Intersection::Successful)
		.def_readwrite("Position", &Plane::Intersection::Position)
		.def_readwrite("T", &Plane::Intersection::T);

	/////////////////////////////////////////////////////
	auto scope3 = py::class_<Sphere>(m, "Sphere");
	scope3.def(py::init<const Vector3f&, float>())
		.def_property("position", &Sphere::position, &Sphere::setPosition)
		.def_property("radius", &Sphere::radius, &Sphere::setRadius)
		.def_property_readonly("volume", &Sphere::volume)
		.def_property_readonly("surfaceArea", &Sphere::surfaceArea)
		.def("isValid", &Sphere::isValid)
		.def("contains", &Sphere::contains)
		.def("intersects", &Sphere::intersects)
		.def("combine", (void (Sphere::*)(const Vector3f&)) & Sphere::combine)
		.def("combined", (Sphere(Sphere::*)(const Vector3f&) const) & Sphere::combined)
		.def("combine", (void (Sphere::*)(const Sphere&)) & Sphere::combine)
		.def("combined", (Sphere(Sphere::*)(const Sphere&) const) & Sphere::combined);

	py::class_<Sphere::Intersection>(scope3, "Intersection")
		.def_readwrite("Successful", &Sphere::Intersection::Successful)
		.def_readwrite("Position", &Sphere::Intersection::Position)
		.def_readwrite("T", &Sphere::Intersection::T);
}
}