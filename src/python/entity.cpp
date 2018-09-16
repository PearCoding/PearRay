#include "entity/VirtualEntity.h"
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
	py::class_<VirtualEntity, std::shared_ptr<VirtualEntity>>(m, "VirtualEntity")
		.def(py::init<uint32, std::string>())
		.def_property_readonly("id", &VirtualEntity::id)
		.def_property("name", &VirtualEntity::name, &VirtualEntity::setName)
		.def_property_readonly("type", &VirtualEntity::type)
		.def_property("flags", &VirtualEntity::flags, &VirtualEntity::setFlags)
		.def_property_readonly("transform", &VirtualEntity::transform)
		.def_property_readonly("invTransform", &VirtualEntity::invTransform)
		.def_property_readonly("directionMatrix", &VirtualEntity::directionMatrix)
		.def_property_readonly("invDirectionMatrix", &VirtualEntity::invDirectionMatrix)
		.def_property_readonly("frozen", &VirtualEntity::isFrozen)
		.def("dumpInformation", &VirtualEntity::dumpInformation);

	py::enum_<EntityFlags>(m, "EntityFlags")
		.value("DEBUG", EF_Debug)
		.value("LOCALAREA", EF_LocalArea);

	py::class_<RenderEntity, std::shared_ptr<RenderEntity>, VirtualEntity>(m, "RenderEntity")
		//.def(py::init<uint32, std::string>())
		.def("isLight", &RenderEntity::isLight)
		.def("surfaceArea", &RenderEntity::surfaceArea)
		.def("isCollidable", &RenderEntity::isCollidable)
		.def("collisionCost", &RenderEntity::collisionCost)
		.def("localBoundingBox", &RenderEntity::localBoundingBox)
		.def("worldBoundingBox", &RenderEntity::worldBoundingBox);
}
} // namespace PRPY