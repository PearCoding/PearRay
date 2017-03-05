#include <boost/python.hpp>
#include "renderer/RenderStatus.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class RenderStatusWrap : public RenderStatus, public bpy::wrapper<RenderStatus>
    {
    public:
        RenderStatusWrap(const boost::reference_wrapper<const RenderStatus>::type& other) :
            RenderStatus(other) {}
        
        bpy::object getField_Py(const std::string& unique_name) const
        {
            Variant var = getField(unique_name);
            switch(var.type())
            {
            case Variant::T_Bool:
                return bpy::object(var.getBool());
            case Variant::T_Int:
                return bpy::object(var.getInt());
            case Variant::T_UInt:
                return bpy::object(var.getUInt());
            case Variant::T_Number:
                return bpy::object(var.getNumber());
            case Variant::T_String:
                return bpy::object(var.getString());
            }
            return bpy::object();
        }

        bpy::dict toDict() const
        {
            bpy::dict d;
            for(auto it = begin(); it != end(); ++it)
                d.setdefault(it->first, getField_Py(it->first));

            return d;
        }
    };

    void setup_status()
    {
        bpy::class_<RenderStatusWrap>("RenderStatus", bpy::no_init)
            .add_property("percentage", &RenderStatus::percentage, &RenderStatus::setPercentage)
            .def("toDict", &RenderStatusWrap::toDict)
            .def("__getitem__", &RenderStatusWrap::getField_Py)
            .def("__setitem__", &RenderStatus::setField)
            .def("__contains__", &RenderStatus::hasField)
        ;
    }
}