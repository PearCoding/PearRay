#include "pymath.h"

namespace PRPY
{
    bpy::object convert2D(const PM::vec2& v)
    {
        return bpy::make_tuple(PM::pm_GetX(v), PM::pm_GetY(v));
    }

    PM::vec2 to2D(const bpy::object& obj)
    {
        bpy::tuple tpl = bpy::extract<bpy::tuple>(obj);
        return PM::pm_Set(bpy::extract<float>(tpl[0]), bpy::extract<float>(tpl[1]));
    }

    bpy::object convert3D(const PM::vec3& v)
    {
        return bpy::make_tuple(PM::pm_GetX(v), PM::pm_GetY(v), PM::pm_GetZ(v));
    }

    PM::vec3 to3D(const bpy::object& obj)
    {
        bpy::tuple tpl = bpy::extract<bpy::tuple>(obj);
        return PM::pm_Set(bpy::extract<float>(tpl[0]),
            bpy::extract<float>(tpl[1]),
            bpy::extract<float>(tpl[2]));
    }

    bpy::object convert4D(const PM::vec4& v)
    {
        return bpy::make_tuple(PM::pm_GetX(v), PM::pm_GetY(v), PM::pm_GetZ(v), PM::pm_GetW(v));
    }

    PM::vec4 to4D(const bpy::object& obj)
    {
        bpy::tuple tpl = bpy::extract<bpy::tuple>(obj);
        return PM::pm_Set(bpy::extract<float>(tpl[0]),
            bpy::extract<float>(tpl[1]),
            bpy::extract<float>(tpl[2]),
            bpy::extract<float>(tpl[3]));
    }

    bpy::object convertMat(const PM::mat4& m)
    {
        return bpy::make_tuple(
            convert4D(m.v[0]), convert4D(m.v[1]), convert4D(m.v[2]), convert4D(m.v[3]));
    }

    PM::mat4 toMat(const bpy::object& obj)
    {
        bpy::tuple tpl = bpy::extract<bpy::tuple>(obj);
        PM::mat4 mat;
        mat.v[0] = to4D(tpl[0]);
        mat.v[1] = to4D(tpl[1]);
        mat.v[2] = to4D(tpl[2]);
        mat.v[3] = to4D(tpl[3]);
        return mat;
    }

    bpy::object convertQuat(const PM::quat& v)
    {
        return convert4D(v);
    }

    PM::quat toQuat(const bpy::object& obj)
    {
        return to4D(obj);
    }
}