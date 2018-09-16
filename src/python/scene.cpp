#include "camera/Camera.h"
#include "entity/VirtualEntity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"
#include "renderer/RenderContext.h"
#include "scene/Scene.h"
#include "scene/SceneFactory.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_scene(py::module& m)
{
	py::class_<SceneFactory>(m, "SceneFactory")
		.def(py::init<std::string>())
		.def_property("name", &SceneFactory::name, &SceneFactory::setName)
		.def_property_readonly("entities", &SceneFactory::entities)
		.def_property_readonly("renderEntities", &SceneFactory::renderEntities)
		.def("addEntity", &SceneFactory::addEntity)
		.def("removeEntity", &SceneFactory::removeEntity)
		.def("getEntity", &SceneFactory::getEntity)
		.def("addInfiniteLight", &SceneFactory::addInfiniteLight)
		.def("removeInfiniteLight", &SceneFactory::removeInfiniteLight)
		.def_property_readonly("infiniteLights", &SceneFactory::infiniteLights)
		.def_property("activeCamera", &SceneFactory::activeCamera, &SceneFactory::setActiveCamera)
		.def("clear", &SceneFactory::clear)
		.def("create", &SceneFactory::create);

	py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
		.def_property_readonly("name", &Scene::name)
		.def_property_readonly("entities", &Scene::entities)
		.def_property_readonly("renderEntities", &Scene::renderEntities)
		.def("getEntity", &Scene::getEntity)
		.def_property_readonly("infiniteLights", &Scene::infiniteLights)
		.def_property_readonly("activeCamera", &Scene::activeCamera)
		.def_property_readonly("boundingBox", &Scene::boundingBox);
}
}