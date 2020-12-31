#include "entity/IEntity.h"
#include "entity/ITransformable.h"

#include "pypearray.h"

using namespace PR;

namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_entity(py::module& m)
{
	py::class_<ITransformable, std::shared_ptr<ITransformable>>(m, "ITransformable")
		.def(py::init<std::string, Transformf>())
		.def_property_readonly("name", &ITransformable::name)
		.def_property_readonly("type", &ITransformable::type)
		.def_property_readonly("transform", &ITransformable::transform)
		.def_property_readonly("invTransform", &ITransformable::invTransform)
		.def_property_readonly("normalMatrix", &ITransformable::normalMatrix)
		.def_property_readonly("invNormalMatrix", &ITransformable::invNormalMatrix)
		.def("dumpInformation", &ITransformable::dumpInformation);

	py::class_<IEntity, std::shared_ptr<IEntity>, ITransformable>(m, "IEntity")
		.def("hasEmission", &IEntity::hasEmission)
		.def("localSurfaceArea", &IEntity::localSurfaceArea)
		.def("worldSurfaceArea", &IEntity::worldSurfaceArea)
		.def("isCollidable", &IEntity::isCollidable)
		.def("collisionCost", &IEntity::collisionCost)
		.def("localBoundingBox", &IEntity::localBoundingBox)
		.def("worldBoundingBox", &IEntity::worldBoundingBox);
}
} // namespace PRPY