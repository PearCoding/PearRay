#include <boost/python.hpp>
#include "ray/Ray.h"
#include "material/Material.h"
#include "shader/FaceSample.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "sampler/Sampler.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class EntityWrap : public Entity, public bpy::wrapper<Entity>
    {
    public:
        EntityWrap(uint32 id, const std::string& name) : Entity(id, name) {}

        inline virtual void onFreeze() override
        {
            if (bpy::override f = this->get_override("onFreeze"))
                f();
            Entity::onFreeze();
        }
    };

    class RenderEntityWrap : public RenderEntity, public bpy::wrapper<RenderEntity>
    {
    public:
        RenderEntityWrap(uint32 id, const std::string& name) : RenderEntity(id, name) {}

        bool isLight() const override
        {
            return this->get_override("isLight")();
        }

        float surfaceArea(Material* m) const override
        {
            return this->get_override("surfaceArea")(m);
        }

        bool isCollidable() const override
        {
            if(bpy::override f = this->get_override("isCollidable"))
                return f();
            return RenderEntity::isCollidable();
        }

        bool isCollidable_PyDef() const { return RenderEntity::isCollidable(); }

        float collisionCost() const override
        {
            if(bpy::override f = this->get_override("collisionCost"))
                return f();
            return RenderEntity::collisionCost();
        }

        float collisionCost_PyDef() const { return RenderEntity::collisionCost(); }

        BoundingBox localBoundingBox() const override
        {
            return this->get_override("localBoundingBox")();
        }

        bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override
        {
            bpy::tuple tpl = checkCollision_Py(ray);
            collisionPoint = bpy::extract<FaceSample>(tpl[1]);
            return bpy::extract<bool>(tpl[0]);
        }

        bpy::tuple checkCollision_Py(const Ray& ray) const
        {
            return this->get_override("checkCollision")(ray);
        }

        FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override
        {
            bpy::tuple tpl = getRandomFacePoint_Py(sampler, sample);
            pdf = bpy::extract<float>(tpl[1]);
            return bpy::extract<FaceSample>(tpl[0]);
        }

        bpy::tuple getRandomFacePoint_Py(Sampler& sampler, uint32 sample) const
        {
            return this->get_override("getRandomFacePoint")(sampler, sample);
        }
    };

    void setup_entity()
    {
        bpy::class_<EntityWrap, boost::noncopyable>("Entity", bpy::init<uint32, std::string>())
        .add_property("id", &Entity::id)
        .add_property("name", &Entity::name, &Entity::setName)
        .add_property("type", &Entity::type)
        .add_property("flags", &Entity::flags, &Entity::setFlags)
        .add_property("position", &Entity::position, &Entity::setPosition)
        .add_property("scale", &Entity::scale, &Entity::setScale)
        .add_property("rotation", &Entity::rotation, &Entity::setRotation)
        .add_property("matrix", &Entity::matrix)
        .add_property("invMatrix", &Entity::invMatrix)
        .add_property("directionMatrix", &Entity::directionMatrix)
        .add_property("invDirectionMatrix", &Entity::invDirectionMatrix)
        .def("__str__", &Entity::toString)
        .def("freeze", &Entity::freeze)
        .add_property("frozen", &Entity::isFrozen)
        .def("onFreeze", &EntityWrap::onFreeze)
        .def("invalidateCache", &Entity::invalidateCache)
        .def("dumpInformation", &Entity::dumpInformation)
        ;

        bpy::enum_<EntityFlags>("EntityFlags")
        .value("Debug", EF_Debug)
        .value("LocalArea", EF_LocalArea)
        .value("ScaleLight", EF_ScaleLight)
        ;

        bpy::class_<RenderEntityWrap, bpy::bases<Entity>, boost::noncopyable>("RenderEntity", bpy::init<uint32, std::string>())
        .def("isLight", bpy::pure_virtual(&RenderEntity::isLight))
        .def("surfaceArea", bpy::pure_virtual(&RenderEntity::surfaceArea))
        .def("isCollidable", &RenderEntity::isCollidable, &RenderEntityWrap::isCollidable_PyDef)
        .def("collisionCost", &RenderEntity::collisionCost, &RenderEntityWrap::collisionCost_PyDef)
        .def("localBoundingBox", bpy::pure_virtual(&RenderEntity::localBoundingBox))
        .def("worldBoundingBox", &RenderEntity::worldBoundingBox)
        .def("checkCollision", bpy::pure_virtual(&RenderEntityWrap::checkCollision_Py))
        .def("getRandomFacePoint", bpy::pure_virtual(&RenderEntityWrap::getRandomFacePoint_Py))
        ;

        bpy::register_ptr_to_python<std::shared_ptr<Entity> >();
        bpy::register_ptr_to_python<std::shared_ptr<RenderEntity> >();
    }
}