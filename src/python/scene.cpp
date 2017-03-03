#include <boost/python.hpp>
#include "scene/Scene.h"
#include "renderer/RenderContext.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    void setup_scene()
    {
        bpy::class_<Scene, boost::noncopyable>("Scene", bpy::init<std::string>())
        .add_property("name", &Scene::name, &Scene::setName)
        .add_property("entities",
            bpy::make_function(&Scene::entities,
                bpy::return_internal_reference<>()))
        .add_property("renderEntities",
            bpy::make_function(&Scene::renderEntities,
                bpy::return_internal_reference<>()))
        .def("addEntity", &Scene::addEntity)
        .def("removeEntity", &Scene::removeEntity)
        .def("getEntity", &Scene::getEntity)
        .def("addInfiniteLight", &Scene::addInfiniteLight)
        .def("removeInfiniteLight", &Scene::removeInfiniteLight)
        .add_property("infiniteLights",
            bpy::make_function(&Scene::infiniteLights,
                bpy::return_internal_reference<>()))
        .def("clear", &Scene::clear)
        .def("buildTree", &Scene::buildTree)
        .def("freeze", &Scene::freeze)
        .def("setup", &Scene::setup)
        .add_property("boundingBox", &Scene::boundingBox)
        ;
    }
}