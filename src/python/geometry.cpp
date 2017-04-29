#include <boost/python.hpp>
#include "ray/Ray.h"
#include "geometry/BoundingBox.h"
#include "geometry/Face.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class BoundingBoxWrap : public BoundingBox, public bpy::wrapper<BoundingBox>
    {
    public:
        BoundingBoxWrap() : BoundingBox() {}
		BoundingBoxWrap(const PM::vec3& upperbound, const PM::vec3& lowerbound) :
            BoundingBox(upperbound, lowerbound) {}
		BoundingBoxWrap(float width, float height, float depth) :
            BoundingBox(width, height, depth) {}

        BoundingBoxWrap(const boost::reference_wrapper<const BoundingBox>::type& other) :
            BoundingBox(other) {}

        // Better names?
        inline bpy::tuple intersects1_Py(const Ray& ray) const {
            float t;
            bool b = intersects(ray, t);
            return bpy::make_tuple(b, t);
        }

        inline bpy::tuple intersects2_Py(const Ray& ray) const {
            float t; PM::vec3 collisionPoint;
            bool b = intersects(ray, collisionPoint, t);
            return bpy::make_tuple(b, collisionPoint, t);
        }

        inline bpy::tuple intersects3_Py(const Ray& ray) const {
            float t; PM::vec3 collisionPoint; BoundingBox::FaceSide side;
            bool b = intersects(ray, collisionPoint, t, side);
            return bpy::make_tuple(b, collisionPoint, t, side);
        }
    };

    class PlaneWrap : public Plane, public bpy::wrapper<Plane>
    {
    public:
        PlaneWrap() : Plane() {}
		PlaneWrap(const PM::vec3& pos, const PM::vec3& xAxis, const PM::vec3& yAxis) :
            Plane(pos, xAxis, yAxis) {}
		PlaneWrap(float width, float height) :
            Plane(width, height) {}

        PlaneWrap(const boost::reference_wrapper<const Plane>::type& other) :
            Plane(other) {}

        inline bpy::tuple intersects_Py(const Ray& ray) const {
            PM::vec3 pos; float t, u, v;
            bool b = intersects(ray, pos, t, u, v);
            return bpy::make_tuple(b, pos, t, u, v);
        }

        inline bpy::tuple project_Py(const PM::vec3& p) const {
            float u, v;
            project(p, u, v);
            return bpy::make_tuple(u, v);
        }
    };

    class SphereWrap : public Sphere, public bpy::wrapper<Sphere>
    {
    public:
        SphereWrap() : Sphere() {}
		SphereWrap(const PM::vec3& pos, float radius) :
            Sphere(pos, radius) {}

        SphereWrap(const boost::reference_wrapper<const Sphere>::type& other) :
            Sphere(other) {}

        inline bpy::tuple intersects_Py(const Ray& ray) const {
            PM::vec3 pos; float t;
            bool b = intersects(ray, pos, t);
            return bpy::make_tuple(b, pos, t);
        }
    };

    void setup_geometry()
    {
        {bpy::scope scope = bpy::class_<BoundingBoxWrap>("BoundingBox")
            .def(bpy::init<const PM::vec3&, const PM::vec3&>())
            .def(bpy::init<float, float, float>())
            .add_property("upperBound", &BoundingBox::upperBound, &BoundingBox::setUpperBound)
            .add_property("lowerBound", &BoundingBox::lowerBound, &BoundingBox::setLowerBound)
            .add_property("center", &BoundingBox::center)
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
            .def("put", &BoundingBoxWrap::put)
            .def("shift", &BoundingBox::shift)
            .def("putted", &BoundingBox::putted)
            .def("shifted", &BoundingBox::shifted)
            .def("getFace", &BoundingBox::getFace)
        ;
        
        bpy::enum_<BoundingBox::FaceSide>("FaceSide")
        .value("LEFT", BoundingBox::FS_Left)
        .value("RIGHT", BoundingBox::FS_Right)
        .value("TOP", BoundingBox::FS_Top)
        .value("BOTTOM", BoundingBox::FS_Bottom)
        .value("FRONT", BoundingBox::FS_Front)
        .value("BACK", BoundingBox::FS_Back)
        ;
        }// End of scope

        bpy::class_<PlaneWrap>("Plane")
            .def(bpy::init<const PM::vec3&, const PM::vec3&, const PM::vec3&>())
            .def(bpy::init<float, float>())
            .add_property("position", &Plane::position, &Plane::setPosition)
            .add_property("xAxis", &Plane::xAxis, &Plane::setXAxis)
            .add_property("yAxis", &Plane::yAxis, &Plane::setYAxis)
            .add_property("center", &Plane::center)
            .add_property("normal", &Plane::normal)
            .add_property("surfaceArea", &Plane::surfaceArea)
            .def("setAxis", &Plane::setAxis)
            .def("isValid", &Plane::isValid)
            .def("contains", &Plane::contains)
            .def("intersects", &PlaneWrap::intersects_Py)
            .def("project", &PlaneWrap::project_Py)
            .add_property("boundingBox", &Plane::toBoundingBox)
            .add_property("localBoundingBox", &Plane::toLocalBoundingBox)
        ;

        bpy::class_<SphereWrap>("Sphere")
            .def(bpy::init<const PM::vec3&, float>())
            .add_property("position", &Sphere::position, &Sphere::setPosition)
            .add_property("radius", &Sphere::radius, &Sphere::setRadius)
            .add_property("volume", &Sphere::volume)
            .add_property("surfaceArea", &Sphere::surfaceArea)
            .def("isValid", &Sphere::isValid)
            .def("contains", &Sphere::contains)
            .def("intersects", &SphereWrap::intersects_Py)
            .def("put", &Sphere::put)
            .def("putted", &Sphere::putted)
            .def("combine", &Sphere::combine)
            .def("combined", &Sphere::combined)
        ;
    }
}