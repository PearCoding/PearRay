#include "pymath.h"
#include "entity/Entity.h"

using namespace PR;
namespace PRPY
{
    class EntityWrap : public Entity, public bpy::wrapper<Entity>
    {
    public:
        EntityWrap(uint32 id, const std::string& name) : Entity(id, name) {}

        inline void setPosition_Py(const bpy::object& pos) { setPosition(to3D(pos)); }
		inline bpy::object position_Py() const { return convert3D(position()); }

		inline void setScale_Py(const bpy::object& s) { setScale(to3D(s)); }
		inline bpy::object scale_Py() const { return convert3D(scale()); }

		inline void setRotation_Py(const bpy::object& quat) { setRotation(toQuat(quat)); }
		inline bpy::object rotation_Py() const { return convertQuat(rotation()); }

		inline bpy::object matrix_Py() const { return convertMat(matrix()); }
		inline bpy::object invMatrix_Py() const { return convertMat(invMatrix()); }

		inline bpy::object directionMatrix_Py() const { return convertMat(directionMatrix()); }
		inline bpy::object invDirectionMatrix_Py() const { return convertMat(invDirectionMatrix()); }

        inline virtual void onFreeze() override
        {
            if (bpy::override f = this->get_override("onFreeze"))
                f();
            Entity::onFreeze();
        }
    };

    void setup_entity()
    {
        bpy::class_<EntityWrap, boost::noncopyable>("Entity", bpy::init<uint32, std::string>())
        .add_property("id", &Entity::id)
        .add_property("name", &Entity::name, &Entity::setName)
        .add_property("type", &Entity::type)
        .add_property("isRenderable", &Entity::isRenderable)
        .add_property("flags", &Entity::flags, &Entity::setFlags)
        .add_property("position", &EntityWrap::position_Py, &EntityWrap::setPosition_Py)
        .add_property("scale", &EntityWrap::scale_Py, &EntityWrap::setScale_Py)
        .add_property("rotation", &EntityWrap::rotation_Py, &EntityWrap::setRotation_Py)
        .add_property("matrix", &EntityWrap::matrix_Py)
        .add_property("invMatrix", &EntityWrap::invMatrix_Py)
        .add_property("directionMatrix", &EntityWrap::directionMatrix_Py)
        .add_property("invDirectionMatrix", &EntityWrap::invDirectionMatrix_Py)
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
    }
}