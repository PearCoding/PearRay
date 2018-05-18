#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"
#include "sampler/Sampler.h"
#include "shader/FacePoint.h"

#include "pypearray.h"

using namespace PR;

namespace PRPY {

void setup_entity(py::module& m)
{
	py::class_<Entity, std::shared_ptr<Entity>>(m, "Entity")
		.def(py::init<uint32, std::string>())
		.def_property_readonly("id", &Entity::id)
		.def_property("name", &Entity::name, &Entity::setName)
		.def_property_readonly("type", &Entity::type)
		.def_property("flags", &Entity::flags, &Entity::setFlags)
		.def_property_readonly("transform", &Entity::transform)
		.def_property_readonly("invTransform", &Entity::invTransform)
		.def_property_readonly("directionMatrix", &Entity::directionMatrix)
		.def_property_readonly("invDirectionMatrix", &Entity::invDirectionMatrix)
		.def_property_readonly("frozen", &Entity::isFrozen)
		.def("dumpInformation", &Entity::dumpInformation);

	py::enum_<EntityFlags>(m, "EntityFlags")
		.value("DEBUG", EF_Debug)
		.value("LOCALAREA", EF_LocalArea);

	py::class_<RenderEntity, std::shared_ptr<RenderEntity>, Entity>(m, "RenderEntity")
		//.def(py::init<uint32, std::string>())
		.def("isLight", &RenderEntity::isLight)
		.def("surfaceArea", &RenderEntity::surfaceArea)
		.def("isCollidable", &RenderEntity::isCollidable)
		.def("collisionCost", &RenderEntity::collisionCost)
		.def("localBoundingBox", &RenderEntity::localBoundingBox)
		.def("worldBoundingBox", &RenderEntity::worldBoundingBox);
}
} // namespace PRPY