#include "camera/Camera.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"
#include "renderer/RenderContext.h"
#include "scene/Scene.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_scene(py::module& m)
{
	py::class_<Scene>(m, "Scene")
		.def(py::init<std::string>())
		.def_property("name", &Scene::name, &Scene::setName)
		.def_property_readonly("entities", &Scene::entities)
		.def_property_readonly("renderEntities", &Scene::renderEntities)
		.def("addEntity", &Scene::addEntity)
		.def("removeEntity", &Scene::removeEntity)
		.def("getEntity", &Scene::getEntity)
		.def("addInfiniteLight", &Scene::addInfiniteLight)
		.def("removeInfiniteLight", &Scene::removeInfiniteLight)
		.def_property_readonly("infiniteLights", &Scene::infiniteLights)
		.def_property("activeCamera", &Scene::activeCamera, &Scene::setActiveCamera)
		.def("clear", &Scene::clear)
		.def("buildTree", &Scene::buildTree)
		.def("freeze", &Scene::freeze)
		.def("setup", &Scene::setup)
		.def_property_readonly("boundingBox", &Scene::boundingBox);
}
}