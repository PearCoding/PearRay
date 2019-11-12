#include "entity/IEntity.h"
#include "entity/VirtualEntity.h"

#include "pypearray.h"

using namespace PR;

namespace PRPY {

void setup_entity(py::module& m)
{
	py::class_<VirtualEntity, std::shared_ptr<VirtualEntity>>(m, "VirtualEntity")
		.def(py::init<uint32, std::string>())
		.def_property_readonly("id", &VirtualEntity::id)
		.def_property("name", &VirtualEntity::name, &VirtualEntity::setName)
		.def_property_readonly("type", &VirtualEntity::type)
		.def_property("flags", &VirtualEntity::flags, &VirtualEntity::setFlags)
		.def_property_readonly("transform", &VirtualEntity::transform)
		.def_property_readonly("invTransform", &VirtualEntity::invTransform)
		.def_property_readonly("normalMatrix", &VirtualEntity::normalMatrix)
		.def_property_readonly("invNormalMatrix", &VirtualEntity::invNormalMatrix)
		.def_property_readonly("frozen", &VirtualEntity::isFrozen)
		.def("dumpInformation", &VirtualEntity::dumpInformation);

	py::enum_<EntityFlags>(m, "EntityFlags")
		.value("DEBUG", EF_Debug)
		.value("LOCALAREA", EF_LocalArea);

	py::class_<IEntity, std::shared_ptr<IEntity>, VirtualEntity>(m, "IEntity")
		.def("isLight", &IEntity::isLight)
		.def("surfaceArea", &IEntity::surfaceArea)
		.def("isCollidable", &IEntity::isCollidable)
		.def("collisionCost", &IEntity::collisionCost)
		.def("localBoundingBox", &IEntity::localBoundingBox)
		.def("worldBoundingBox", &IEntity::worldBoundingBox);
}
} // namespace PRPY