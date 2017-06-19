#pragma once

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

namespace PRPY {
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

np::ndarray vec2ToPython(const Eigen::Vector2f& v);
Eigen::Vector2f vec2FromPython(const np::ndarray& arr);

np::ndarray ivec2ToPython(const Eigen::Vector2i& v);
Eigen::Vector2i ivec2FromPython(const np::ndarray& arr);

np::ndarray vec3ToPython(const Eigen::Vector3f& v);
Eigen::Vector3f vec3FromPython(const np::ndarray& arr);

np::ndarray vec4ToPython(const Eigen::Vector4f& v);
Eigen::Vector4f vec4FromPython(const np::ndarray& arr);

np::ndarray quatToPython(const Eigen::Quaternionf& v);
Eigen::Quaternionf quatFromPython(const np::ndarray& arr);

np::ndarray aff3ToPython(const Eigen::Affine3f& v);
Eigen::Affine3f aff3FromPython(const np::ndarray& arr);

np::ndarray mat3ToPython(const Eigen::Matrix3f& v);
Eigen::Matrix3f mat3FromPython(const np::ndarray& arr);

np::ndarray mat4ToPython(const Eigen::Matrix4f& v);
Eigen::Matrix4f mat4FromPython(const np::ndarray& arr);

#define _PRPY_WRAP_GET(f, m) \
	np::ndarray f ## _Py() const { return m(f()); }

#define _PRPY_WRAP_SET(f, m) \
	void f ## _Py (const np::ndarray& arr) { f(m(arr)); }

#define PRPY_WRAP_GET_VEC2(f) _PRPY_WRAP_GET(f, vec2ToPython)
#define PRPY_WRAP_SET_VEC2(f) _PRPY_WRAP_SET(f, vec2FromPython)
#define PRPY_WRAP_GET_IVEC2(f) _PRPY_WRAP_GET(f, ivec2ToPython)
#define PRPY_WRAP_SET_IVEC2(f) _PRPY_WRAP_SET(f, ivec2FromPython)
#define PRPY_WRAP_GET_VEC3(f) _PRPY_WRAP_GET(f, vec3ToPython)
#define PRPY_WRAP_SET_VEC3(f) _PRPY_WRAP_SET(f, vec3FromPython)
#define PRPY_WRAP_GET_VEC4(f) _PRPY_WRAP_GET(f, vec4ToPython)
#define PRPY_WRAP_SET_VEC4(f) _PRPY_WRAP_SET(f, vec4FromPython)
#define PRPY_WRAP_GET_QUAT(f) _PRPY_WRAP_GET(f, quatToPython)
#define PRPY_WRAP_SET_QUAT(f) _PRPY_WRAP_SET(f, quatFromPython)
#define PRPY_WRAP_GET_AFF3(f) _PRPY_WRAP_GET(f, aff3ToPython)
#define PRPY_WRAP_SET_AFF3(f) _PRPY_WRAP_SET(f, aff3FromPython)
#define PRPY_WRAP_GET_MAT3(f) _PRPY_WRAP_GET(f, mat3ToPython)
#define PRPY_WRAP_SET_MAT3(f) _PRPY_WRAP_SET(f, mat3FromPython)
#define PRPY_WRAP_GET_MAT4(f) _PRPY_WRAP_GET(f, mat4ToPython)
#define PRPY_WRAP_SET_MAT4(f) _PRPY_WRAP_SET(f, mat4FromPython)
}