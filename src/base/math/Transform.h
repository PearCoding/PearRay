#pragma once

#include "PR_Config.h"
#include "SIMD.h"

namespace PR {

namespace Transform {

constexpr float RayOffsetEpsilon = 0.000001f;
/* Translates original position a little bit
 * in direction of the view to ensure no self intersection.
 */
inline Vector3f safePosition(const Vector3f& pos,
							 const Vector3f& dir,
							 const Vector3f& N)
{
	float d			= (N.cwiseAbs() * RayOffsetEpsilon).sum();
	Vector3f offset = d * N;

	if (dir.dot(N) < 0)
		offset = -offset;
	Vector3f posOff = pos + offset;

	for (int i = 0; i < 3; ++i) {
		if (offset[i] > 0)
			posOff[i] = nextFloatUp(posOff[i]);
		else if (offset[i] < 0)
			posOff[i] = nextFloatDown(posOff[i]);
	}
	return posOff;
}

inline Vector3fv safePosition(const Vector3fv& pos,
							  const Vector3fv& dir,
							  const Vector3fv& N)
{
	vfloat d		 = (N.cwiseAbs() * vfloat(RayOffsetEpsilon)).sum();
	Vector3fv offset = d * N;

	bfloat neg		 = dir.dot(N) < 0;
	offset[0]		 = blend(-offset[0], offset[0], neg);
	offset[1]		 = blend(-offset[1], offset[1], neg);
	offset[2]		 = blend(-offset[2], offset[2], neg);
	Vector3fv posOff = pos + offset;

	for (int i = 0; i < 3; ++i) {
		vfloat add = foreach_assign_v(posOff[i], [&](float val) { return nextFloatUp(val); });
		vfloat sub = foreach_assign_v(posOff[i], [&](float val) { return nextFloatDown(val); });
		posOff[i]  = blend(add,
						   blend(sub, posOff[i], offset[i] < 0),
						   offset[i] > 0);
	}
	return posOff;
}

template <typename T>
inline Vector3t<T> applyVector(const Eigen::Ref<const Eigen::Matrix3f>& m, const Vector3t<T>& p)
{
	T b1 = m(0, 0) * p(0) + m(0, 1) * p(1) + m(0, 2) * p(2);
	T b2 = m(1, 0) * p(0) + m(1, 1) * p(1) + m(1, 2) * p(2);
	T b3 = m(2, 0) * p(0) + m(2, 1) * p(1) + m(2, 2) * p(2);
	return Vector3t<T>(b1, b2, b3);
}

template <typename T>
inline Vector3t<T> applyAffine(const Eigen::Ref<const Eigen::Matrix4f>& m, const Vector3t<T>& p)
{
	T b1 = m(0, 0) * p(0) + m(0, 1) * p(1) + m(0, 2) * p(2) + m(0, 3);
	T b2 = m(1, 0) * p(0) + m(1, 1) * p(1) + m(1, 2) * p(2) + m(1, 3);
	T b3 = m(2, 0) * p(0) + m(2, 1) * p(1) + m(2, 2) * p(2) + m(2, 3);
	return Vector3t<T>(b1, b2, b3);
}

template <typename T>
inline Vector3t<T> apply(const Eigen::Ref<const Eigen::Matrix4f>& m, const Vector3t<T>& p)
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

inline auto orthogonalInverse(const Eigen::Ref<const Eigen::Matrix3f>& mat)
{
	return mat.transpose();
}
} // namespace Transform
} // namespace PR