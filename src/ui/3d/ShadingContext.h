#pragma once

#include "PR_Config.h"

namespace PR {
namespace UI {
struct PR_LIB_UI ShadingContext {
	Matrix4f ObjectMatrix = Matrix4f::Identity();
	Matrix4f ViewMatrix	  = Matrix4f::Identity();
	Matrix4f CameraMatrix = Matrix4f::Identity();
	Matrix4f FullMatrix	  = Matrix4f::Identity();

	ShadingContext() = default;

	inline ShadingContext(const Matrix4f& v, const Matrix4f& c)
		: ObjectMatrix(Matrix4f::Identity())
		, ViewMatrix(v)
		, CameraMatrix(c)
		, FullMatrix(c * v)
	{
	}

	/// Chain mat to object transformation
	inline ShadingContext chain(const Matrix4f& mat) const
	{
		ShadingContext copy = *this;
		copy.ObjectMatrix	= copy.ObjectMatrix * mat;
		copy.FullMatrix		= copy.FullMatrix * mat;
		return copy;
	}

	/// Prechain mat to object transformation
	inline ShadingContext prechain(const Matrix4f& mat) const
	{
		ShadingContext copy = *this;
		copy.ObjectMatrix	= mat * copy.ObjectMatrix;
		copy.recalcFullMatrix();
		return copy;
	}

	inline Matrix3f computeNormalMatrix() const
	{
		return ((ViewMatrix * ObjectMatrix).block<3, 3>(0, 0)).inverse().transpose();
	}

	inline void recalcFullMatrix()
	{
		FullMatrix = CameraMatrix * ViewMatrix * ObjectMatrix;
	}
};
} // namespace UI
} // namespace PR