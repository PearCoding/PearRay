#include "entity/IEntity.h"
#include "entity/ITransformable.h"

#include "pypearray.h"

using namespace PR;

namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_entity(py::module& m)
{
	py::class_<ITransformable, std::shared_ptr<ITransformable>>(m, "ITransformable")
		.def(py::init<uint32, std::string>())
		.def_property_readonly("id", &ITransformable::id)
		.def_property("name", &ITransformable::name, &ITransformable::setName)
		.def_property_readonly("type", &ITransformable::type)
		.def_property("flags", &ITransformable::flags, &ITransformable::setFlags)
		.def_property_readonly("transform", &ITransformable::transform)
		.def_property_readonly("invTransform", &ITransformable::invTransform)
		.def_property_readonly("normalMatrix", &ITransformable::normalMatrix)
		.def_property_readonly("invNormalMatrix", &ITransformable::invNormalMatrix)
		.def("dumpInformation", &ITransformable::dumpInformation);

	py::enum_<EntityFlags>(m, "EntityFlags")
		.value("DEBUG", EF_Debug)
		.value("LOCALAREA", EF_LocalArea);

	py::class_<IEntity, std::shared_ptr<IEntity>, ITransformable>(m, "IEntity")
		.def("isLight", &IEntity::isLight)
		.def("surfaceArea", &IEntity::surfaceArea)
		.def("isCollidable", &IEntity::isCollidable)
		.def("collisionCost", &IEntity::collisionCost)
		.def("localBoundingBox", &IEntity::localBoundingBox)
		.def("worldBoundingBox", &IEntity::worldBoundingBox);
}
} // namespace PRPY