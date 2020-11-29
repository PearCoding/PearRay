// IWYU pragma: private, include "entity/ITransformable.h"
namespace PR {
inline std::string ITransformable::name() const
{
	return mName;
}

inline std::string ITransformable::type() const
{
	return "null";
}

inline bool ITransformable::isRenderable() const
{
	return false;
}

inline void ITransformable::setFlags(uint8 f)
{
	mFlags = f;
}

inline uint8 ITransformable::flags() const
{
	return mFlags;
}

inline const Transformf& ITransformable::transform() const
{
	return mTransform;
}

inline const Transformf& ITransformable::invTransform() const
{
	return mInvTransformCache;
}

inline float ITransformable::volumeScalefactor() const
{
	return mJacobianDeterminant;
}

inline const Eigen::Matrix3f& ITransformable::normalMatrix() const
{
	return mNormalMatrixCache;
}

inline const Eigen::Matrix3f& ITransformable::invNormalMatrix() const
{
	return mInvNormalMatrixCache;
}
} // namespace PR
