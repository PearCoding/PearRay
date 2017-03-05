#include <boost/python.hpp>
#include "ray/Ray.h"
#include "material/Material.h"
#include "entity/BoundaryEntity.h"
#include "entity/CoordinateAxisEntity.h"
#include "entity/MeshEntity.h"
#include "entity/PlaneEntity.h"
#include "entity/SphereEntity.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    void setup_renderentities()
    {
        bpy::class_<BoundaryEntity, std::shared_ptr<BoundaryEntity>, bpy::bases<RenderEntity>, boost::noncopyable>("BoundaryEntity", bpy::init<uint32, std::string, const BoundingBox&>())
        .add_property("material", 
            bpy::make_function(&BoundaryEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &BoundaryEntity::setMaterial)
        .def("setBoundingBox", &BoundaryEntity::setBoundingBox)
        ;
        bpy::implicitly_convertible<std::shared_ptr<BoundaryEntity>, std::shared_ptr<RenderEntity> >();

        bpy::class_<CoordinateAxisEntity, std::shared_ptr<CoordinateAxisEntity>, bpy::bases<RenderEntity>, boost::noncopyable>("CoordinateAxisEntity", bpy::init<uint32, std::string>())
        .add_property("xMaterial", 
            bpy::make_function(&CoordinateAxisEntity::xMaterial, bpy::return_value_policy<bpy::copy_const_reference >()),
            &CoordinateAxisEntity::setXMaterial)
        .add_property("yMaterial", 
            bpy::make_function(&CoordinateAxisEntity::yMaterial, bpy::return_value_policy<bpy::copy_const_reference >()),
            &CoordinateAxisEntity::setYMaterial)
        .add_property("zMaterial", 
            bpy::make_function(&CoordinateAxisEntity::zMaterial, bpy::return_value_policy<bpy::copy_const_reference >()),
            &CoordinateAxisEntity::setZMaterial)
        .add_property("axisLength", &CoordinateAxisEntity::axisLength, &CoordinateAxisEntity::setAxisLength)
        .add_property("axisThickness", &CoordinateAxisEntity::axisThickness, &CoordinateAxisEntity::setAxisThickness)
        ;
        bpy::implicitly_convertible<std::shared_ptr<CoordinateAxisEntity>, std::shared_ptr<RenderEntity> >();

        bpy::class_<MeshEntity, std::shared_ptr<MeshEntity>, bpy::bases<RenderEntity>, boost::noncopyable>("MeshEntity", bpy::init<uint32, std::string>())
        /*.add_property("mesh", 
            bpy::make_function(&MeshEntity::mesh, bpy::return_value_policy<bpy::copy_const_reference >()),
            &MeshEntity::setMesh)*/
        .def("reserveMaterialSlots", &MeshEntity::reserveMaterialSlots)
        .def("setMaterial", &MeshEntity::setMaterial)
        .def("material", &MeshEntity::material)
        ;
        bpy::implicitly_convertible<std::shared_ptr<MeshEntity>, std::shared_ptr<RenderEntity> >();

        bpy::class_<PlaneEntity, std::shared_ptr<PlaneEntity>, bpy::bases<RenderEntity>, boost::noncopyable>("PlaneEntity", bpy::init<uint32, std::string, const Plane&>())
        .add_property("material", 
            bpy::make_function(&PlaneEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &PlaneEntity::setMaterial)
        .add_property("plane",  
            bpy::make_function(&PlaneEntity::plane, bpy::return_value_policy<bpy::copy_const_reference >()),
            &PlaneEntity::setPlane)
        ;
        bpy::implicitly_convertible<std::shared_ptr<PlaneEntity>, std::shared_ptr<RenderEntity> >();

        bpy::class_<SphereEntity, std::shared_ptr<SphereEntity>, bpy::bases<RenderEntity>, boost::noncopyable>("SphereEntity", bpy::init<uint32, std::string, float>())
        .add_property("material", 
            bpy::make_function(&SphereEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &SphereEntity::setMaterial)
        .add_property("radius", &SphereEntity::radius, &SphereEntity::setRadius)
        ;
        bpy::implicitly_convertible<std::shared_ptr<SphereEntity>, std::shared_ptr<RenderEntity> >();
    }
}