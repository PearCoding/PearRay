#include "scene/Scene.h"
#include "camera/ICamera.h"
#include "entity/IEntity.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_scene(py::module& m)
{
	py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
		.def_property_readonly("entities", &Scene::entities)
		.def_property_readonly("activeCamera", &Scene::activeCamera)
		.def_property_readonly("boundingBox", &Scene::boundingBox);
}
} // namespace PRPY