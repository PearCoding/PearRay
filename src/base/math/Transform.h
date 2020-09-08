#pragma once

#include "PR_Config.h"

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

	PR_OPT_LOOP
	for (int i = 0; i < 3; ++i) {
		if (offset[i] > 0)
			posOff[i] = nextFloatUp(posOff[i]);
		else if (offset[i] < 0)
			posOff[i] = nextFloatDown(posOff[i]);
	}
	return posOff;
}

inline Vector3f applyVector(const Eigen::Ref<const Eigen::Matrix3f>& m, const Vector3f& p)
{
	return m * p;
}

inline Vector3f applyAffine(const Eigen::Ref<const Eigen::Matrix4f>& m, const Vector3f& p)
{
	return m.block<3, 3>(0, 0) * p + m.block<3, 1>(0, 3);
}

inline Vector3f apply(const Eigen::Ref<const Eigen::Matrix4f>& m, const Vector3f& p)
{
	Vector3f b	= applyAffine(m, p);
	float denom = m.block<1, 3>(3, 0).dot(p) + m(3, 3);
	return b / denom;
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