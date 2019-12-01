#pragma once

#include "PR_Config.h"

namespace PR {

namespace Transform {
/* Translates original position a little bit
 * in direction of the view to ensure no self intersection.
 */
inline Vector3f safePosition(const Vector3f& pos,
							 const Vector3f& dir)
{
	constexpr float RayOffsetEpsilon = 0.000001f;
	Vector3f off					 = dir * RayOffsetEpsilon;
	Vector3f posOff					 = pos + off;

	for (int i = 0; i < 3; ++i) {
		if (off(i) > 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::max());
		else if (off(i) < 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::lowest());
	}

	return posOff;
}

inline Vector3fv safePosition(const Vector3fv& pos,
							  const Vector3fv& dir)
{
	// FIXME: A bit lackluster....
	constexpr float RayOffsetEpsilon = 0.000001f;
	return pos + dir * vfloat(RayOffsetEpsilon);
}

template <typename T>
inline Vector3t<T> apply(const Eigen::Matrix3f& m, const Vector3t<T>& p)
{
	T b1 = m(0, 0) * p(0) + m(0, 1) * p(1) + m(0, 2) * p(2);
	T b2 = m(1, 0) * p(0) + m(1, 1) * p(1) + m(1, 2) * p(2);
	T b3 = m(2, 0) * p(0) + m(2, 1) * p(1) + m(2, 2) * p(2);
	return Vector3t<T>(b1, b2, b3);
}

template <typename T>
inline Vector3t<T> applyAffine(const Eigen::Matrix4f& m, const Vector3t<T>& p)
{
	T b1 = m(0, 0) * p(0) + m(0, 1) * p(1) + m(0, 2) * p(2) + m(0, 3);
	T b2 = m(1, 0) * p(0) + m(1, 1) * p(1) + m(1, 2) * p(2) + m(1, 3);
	T b3 = m(2, 0) * p(0) + m(2, 1) * p(1) + m(2, 2) * p(2) + m(2, 3);
	return Vector3t<T>(b1, b2, b3);
}

template <typename T>
inline Vector3t<T> apply(const Eigen::Matrix4f& m, const Vector3t<T>& p)
{
	Vector3t<T> b = applyAffine(m, p);
	return b / (m(3, 0) * p(0) + m(3, 1) * p(1) + m(3, 2) * p(2) + m(3, 3));
}

inline Eigen::Matrix3f orthogonalMatrix(const Vector3f& c0, const Vector3f& c1, const Vector3f& c2)
{
	Eigen::Matrix3f mat;
	mat.col(0) = c0;
	mat.col(1) = c1;
	mat.col(2) = c2;

	return mat;
}

inline Eigen::Matrix3f orthogonalInverse(const Eigen::Matrix3f& mat)
{
	return mat.transpose();
}
} // namespace Transform
} // namespace PR