#include "entity/BoundaryEntity.h"
#include "entity/CoordinateAxisEntity.h"
#include "entity/MeshEntity.h"
#include "entity/PlaneEntity.h"
#include "entity/SphereEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_renderentities(py::module& m)
{
	py::class_<BoundaryEntity, RenderEntity, std::shared_ptr<BoundaryEntity>>(m, "BoundaryEntity")
		.def(py::init<uint32, std::string, const BoundingBox&>())
		.def_property("material",
					  &BoundaryEntity::material,
					  &BoundaryEntity::setMaterial)
		.def("setBoundingBox", &BoundaryEntity::setBoundingBox);

	py::class_<CoordinateAxisEntity, RenderEntity, std::shared_ptr<CoordinateAxisEntity>>(m, "CoordinateAxisEntity")
		.def(py::init<uint32, std::string>())
		.def_property("xMaterial",
					  &CoordinateAxisEntity::xMaterial,
					  &CoordinateAxisEntity::setXMaterial)
		.def_property("yMaterial",
					  &CoordinateAxisEntity::yMaterial,
					  &CoordinateAxisEntity::setYMaterial)
		.def_property("zMaterial",
					  &CoordinateAxisEntity::zMaterial,
					  &CoordinateAxisEntity::setZMaterial)
		.def_property("axisLength", &CoordinateAxisEntity::axisLength, &CoordinateAxisEntity::setAxisLength)
		.def_property("axisThickness", &CoordinateAxisEntity::axisThickness, &CoordinateAxisEntity::setAxisThickness);

	py::class_<MeshEntity, RenderEntity, std::shared_ptr<MeshEntity>>(m, "MeshEntity")
		.def(py::init<uint32, std::string>())
		/*.def_property("mesh",
					  &MeshEntity::mesh,
					  &MeshEntity::setMesh)*/
		.def("reserveMaterialSlots", &MeshEntity::reserveMaterialSlots)
		.def("setMaterial", &MeshEntity::setMaterial)
		.def("material", &MeshEntity::material);

	py::class_<PlaneEntity, RenderEntity, std::shared_ptr<PlaneEntity>>(m, "PlaneEntity")
		.def(py::init<uint32, std::string, const Plane&>())
		.def_property("material",
					  &PlaneEntity::material,
					  &PlaneEntity::setMaterial)
		.def_property("plane",
					  &PlaneEntity::plane,
					  &PlaneEntity::setPlane);

	py::class_<SphereEntity, RenderEntity, std::shared_ptr<SphereEntity>>(m, "SphereEntity")
		.def(py::init<uint32, std::string, float>())
		.def_property("material",
					  &SphereEntity::material,
					  &SphereEntity::setMaterial)
		.def_property("radius", &SphereEntity::radius, &SphereEntity::setRadius);
}
}