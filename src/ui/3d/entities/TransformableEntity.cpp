#include "TransformableEntity.h"

namespace PR {
namespace UI {
/// Decomposing matrix representing transform of type T*R into it a translation vector and quaternion
inline void decomposeMatrix(const Matrix4f& mat, Vector3f& translation, Quaternionf& rotation)
{
	translation = mat.block<3, 1>(0, 3);
	rotation	= Quaternionf(mat.block<3, 3>(0, 0));
	rotation.normalize();
}

/// Decomposing matrix representing transform of type T*R*S into it a translation vector, a quaternion and a scale vector
inline void decomposeMatrix(const Matrix4f& mat, Vector3f& translation, Quaternionf& rotation, Vector3f& scale)
{
	translation = mat.block<3, 1>(0, 3);

	Matrix3f RS = mat.block<3, 3>(0, 0);
	scale[0]	= Vector3f(RS(0, 0), RS(1, 0), RS(2, 0)).norm();
	scale[1]	= Vector3f(RS(0, 1), RS(1, 1), RS(2, 1)).norm();
	scale[2]	= Vector3f(RS(0, 2), RS(1, 2), RS(2, 2)).norm();

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			RS(j, i) /= scale[i];

	rotation = Quaternionf(RS);
	rotation.normalize();
}

inline Matrix4f fromTranslation(const Vector3f& v)
{
	Matrix4f m = Matrix4f::Identity();
	m(0, 3)	   = v(0);
	m(1, 3)	   = v(1);
	m(2, 3)	   = v(2);
	return m;
}

inline Matrix4f fromScale(const Vector3f& v)
{
	Matrix4f m = Matrix4f::Identity();
	m(0, 0)	   = v(0);
	m(1, 1)	   = v(1);
	m(2, 2)	   = v(2);
	return m;
}

inline Matrix4f fromRotation(const Quaternionf& v)
{
	Matrix4f m			= Matrix4f::Identity();
	m.block<3, 3>(0, 0) = v.toRotationMatrix();
	return m;
}

TransformableEntity::TransformableEntity()
	: mTranslation(0, 0, 0)
	, mRotation(Quaternionf::Identity())
	, mScale(1, 1, 1)
	, mTransformCache()
	, mVisible(true)
{
	cache();
}

void TransformableEntity::setTransform(const Matrix4f& trans)
{
	decomposeMatrix(trans, mTranslation, mRotation, mScale);
	cache();
}

void TransformableEntity::setTransform(const Vector3f& pos, const Quaternionf& rot, const Vector3f& scale)
{
	mTranslation = pos;
	mRotation	 = rot;
	mScale		 = scale;
	cache();
}

void TransformableEntity::setTransform(const Vector3f& pos, const Quaternionf& rot, float scale)
{
	setTransform(pos, rot, Vector3f(scale, scale, scale));
}

void TransformableEntity::setTransform(const Vector3f& pos, const Quaternionf& rot)
{
	mTranslation = pos;
	mRotation	 = rot;
	cache();
}

void TransformableEntity::setTransformNoScale(const Matrix4f& trans)
{
	decomposeMatrix(trans, mTranslation, mRotation);
	mScale = Vector3f(1, 1, 1);
}

Matrix4f TransformableEntity::transformNoScale() const
{
	return fromTranslation(mTranslation) * fromRotation(mRotation);
}

Matrix3f TransformableEntity::normalTransform() const
{
	return transform().block<3, 3>(0, 0).inverse().transpose();
}

Matrix4f TransformableEntity::calcMatrix() const
{
	return fromTranslation(mTranslation) * fromRotation(mRotation) * fromScale(mScale);
}

void TransformableEntity::translate(const Vector3f& v)
{
	mTranslation += v;
	cache();
}

void TransformableEntity::rotate(const Quaternionf& v)
{
	mRotation = mRotation * v.normalized();
	cache();
}

void TransformableEntity::scale(const Vector3f& v)
{
	mScale = mScale.cwiseProduct(v);
	cache();
}

void TransformableEntity::scale(float f)
{
	mScale *= f;
	cache();
}

void TransformableEntity::setTranslation(const Vector3f& v)
{
	mTranslation = v;
	cache();
}

void TransformableEntity::setRotation(const Quaternionf& v)
{
	mRotation = v;
	cache();
}

void TransformableEntity::setScale(const Vector3f& v)
{
	mScale = v;
	cache();
}

void TransformableEntity::setScale(float f)
{
	mScale = Vector3f(f, f, f);
	cache();
}

void TransformableEntity::cache()
{
	mTransformCache = calcMatrix();
}
} // namespace UI
} // namespace PR