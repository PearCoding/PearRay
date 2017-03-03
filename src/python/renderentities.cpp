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
        bpy::class_<BoundaryEntity, bpy::bases<RenderEntity>, boost::noncopyable>("BoundaryEntity", bpy::init<uint32, std::string, const BoundingBox&>())
        .add_property("material", 
            bpy::make_function(&BoundaryEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &BoundaryEntity::setMaterial)
        .def("setBoundingBox", &BoundaryEntity::setBoundingBox)
        ;

        bpy::class_<CoordinateAxisEntity, bpy::bases<RenderEntity>, boost::noncopyable>("CoordinateAxisEntity", bpy::init<uint32, std::string>())
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

        bpy::class_<MeshEntity, bpy::bases<RenderEntity>, boost::noncopyable>("MeshEntity", bpy::init<uint32, std::string>())
        /*.add_property("mesh", 
            bpy::make_function(&MeshEntity::mesh, bpy::return_value_policy<bpy::copy_const_reference >()),
            &MeshEntity::setMesh)*/
        .def("reserveMaterialSlots", &MeshEntity::reserveMaterialSlots)
        .def("setMaterial", &MeshEntity::setMaterial)
        .def("material", &MeshEntity::material)
        ;

        bpy::class_<PlaneEntity, bpy::bases<RenderEntity>, boost::noncopyable>("PlaneEntity", bpy::init<uint32, std::string, const Plane&>())
        .add_property("material", 
            bpy::make_function(&PlaneEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &PlaneEntity::setMaterial)
        .add_property("plane",  
            bpy::make_function(&PlaneEntity::plane, bpy::return_value_policy<bpy::copy_const_reference >()),
            &PlaneEntity::setPlane)
        ;

        bpy::class_<SphereEntity, bpy::bases<RenderEntity>, boost::noncopyable>("SphereEntity", bpy::init<uint32, std::string, float>())
        .add_property("material", 
            bpy::make_function(&SphereEntity::material, bpy::return_value_policy<bpy::copy_const_reference >()),
            &SphereEntity::setMaterial)
        .add_property("radius", &SphereEntity::radius, &SphereEntity::setRadius)
        ;
    }
}