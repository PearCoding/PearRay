#include <boost/python.hpp>
#include "ray/Ray.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    void setup_ray()
    {
        bpy::class_<Ray>("Ray")
        .def(bpy::init<const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, bpy::optional<uint32, float, uint8, uint16> >())
        .add_property("origin", &Ray::origin, &Ray::setOrigin)
        .add_property("direction", &Ray::direction, &Ray::setDirection)
        .add_property("pixel", &Ray::pixel, &Ray::setPixel)
        .add_property("depth", &Ray::depth, &Ray::setDepth)
        .add_property("time", &Ray::time, &Ray::setTime)
        .add_property("wavelength", &Ray::wavelength, &Ray::setWavelength)
        .add_property("flags", &Ray::flags, &Ray::setFlags)

        .def("next", &Ray::next)
        .add_property("safe", bpy::make_function(
            (Ray (*)(const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, uint32, float, uint8, uint16))&Ray::safe))
        .add_property("safe", bpy::make_function(
            (Ray (*)(const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, uint32, float, uint8))&Ray::safe))
        .add_property("safe", bpy::make_function(
            (Ray (*)(const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, uint32, float))&Ray::safe))
        .add_property("safe", bpy::make_function(
            (Ray (*)(const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&, uint32))&Ray::safe))
        .add_property("safe", bpy::make_function(
            (Ray (*)(const Eigen::Vector2i&, const Eigen::Vector3f&, const Eigen::Vector3f&))&Ray::safe))
        ;

        bpy::enum_<RayFlags>("RayFlags")
        .value("LIGHT", RF_Light)
        .value("DEBUG", RF_Debug)
        ;
    }
}