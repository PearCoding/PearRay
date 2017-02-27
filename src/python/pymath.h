#pragma once

#include "PearMath.h"
#include <boost/python.hpp>

namespace PRPY
{
    namespace bpy = boost::python;

    bpy::object convert2D(const PM::vec2& v);
    PM::vec2 to2D(const bpy::object& obj);
    bpy::object convert3D(const PM::vec3& v);
    PM::vec3 to3D(const bpy::object& obj);
    bpy::object convert4D(const PM::vec4& v);
    PM::vec4 to4D(const bpy::object& obj);
    bpy::object convertMat(const PM::mat4& v);
    PM::mat4 toMat(const bpy::object& obj);
    bpy::object convertQuat(const PM::quat& v);
    PM::quat toQuat(const bpy::object& obj);
}