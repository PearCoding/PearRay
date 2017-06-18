#include "npmath.h"

#include "Logger.h"

namespace PRPY {

np::ndarray vec2ToPython(const Eigen::Vector2f& v)
{
	return np::array(bpy::make_tuple(v.x(), v.y()));
}

Eigen::Vector2f vec2FromPython(const np::ndarray& arr)
{
	if (arr.get_nd() == 1 && arr.shape(0) == 2)
		return Eigen::Vector2f(
			bpy::extract<float>(arr[0]),
			bpy::extract<float>(arr[1]));
	else
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Vector2 given."); // Better error report?

	return Eigen::Vector2f();
}

np::ndarray vec3ToPython(const Eigen::Vector3f& v)
{
	return np::array(bpy::make_tuple(v.x(), v.y(), v.z()));
}

Eigen::Vector3f vec3FromPython(const np::ndarray& arr)
{
	if (arr.get_nd() == 1 && arr.shape(0) == 3)
		return Eigen::Vector3f(
			bpy::extract<float>(arr[0]),
			bpy::extract<float>(arr[1]),
			bpy::extract<float>(arr[2]));
	else
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Vector3 given."); // Better error report?

	return Eigen::Vector3f();
}

np::ndarray vec4ToPython(const Eigen::Vector4f& v)
{
	return np::array(bpy::make_tuple(v.x(), v.y(), v.z(), v.w()));
}

Eigen::Vector4f vec4FromPython(const np::ndarray& arr)
{
	if (arr.get_nd() == 1 && arr.shape(0) == 4)
		return Eigen::Vector4f(
			bpy::extract<float>(arr[0]),
			bpy::extract<float>(arr[1]),
			bpy::extract<float>(arr[2]),
			bpy::extract<float>(arr[3]));
	else
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Vector4 given."); // Better error report?

	return Eigen::Vector4f();
}

np::ndarray quatToPython(const Eigen::Quaternionf& v)
{
	return np::array(bpy::make_tuple(v.x(), v.y(), v.z(), v.w()));
}

Eigen::Quaternionf quatFromPython(const np::ndarray& arr)
{
	if (arr.get_nd() == 1 && arr.shape(0) == 4)
		return Eigen::Quaternionf(
			bpy::extract<float>(arr[0]),
			bpy::extract<float>(arr[1]),
			bpy::extract<float>(arr[2]),
			bpy::extract<float>(arr[3]));
	else
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Quaternion given."); // Better error report?

	return Eigen::Quaternionf();
}

np::ndarray aff3ToPython(const Eigen::Affine3f& v)
{
	return mat4ToPython(v.matrix());
}

Eigen::Affine3f aff3FromPython(const np::ndarray& arr)
{
	return Eigen::Affine3f(mat4FromPython(arr));
}

np::ndarray mat3ToPython(const Eigen::Matrix3f& v)
{
	np::ndarray arr = np::empty(bpy::make_tuple(3, 3), np::dtype::get_builtin<float>());

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			arr(i, j) = bpy::object(v(i, j));
		}
	}

	return arr;
}

Eigen::Matrix3f mat3FromPython(const np::ndarray& arr)
{
	Eigen::Matrix3f m;

	if (arr.get_nd() == 2 && arr.shape(0) == 3 && arr.shape(1) == 3) {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				m(i, j) = bpy::extract<float>(arr(i, j));
			}
		}
	} else {
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Matrix3 given."); // Better error report?
	}

	return m;
}

np::ndarray mat4ToPython(const Eigen::Matrix4f& v)
{
	np::ndarray arr = np::empty(bpy::make_tuple(4, 4), np::dtype::get_builtin<float>());

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			arr(i, j) = bpy::object(v(i, j));
		}
	}

	return arr;
}

Eigen::Matrix4f mat4FromPython(const np::ndarray& arr)
{
	Eigen::Matrix4f m;

	if (arr.get_nd() == 2 && arr.shape(0) == 4 && arr.shape(1) == 4) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				m(i, j) = bpy::extract<float>(arr(i, j));
			}
		}
	} else {
		PR_LOGGER.logf(PR::L_Error, PR::M_Script, "Invalid array of type Matrix4 given."); // Better error report?
	}

	return m;
}
}