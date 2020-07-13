// IWYU pragma: private, include "entity/ITransformable.h"
namespace PR {
inline uint32 ITransformable::id() const
{
	return mID;
}

inline void ITransformable::setName(const std::string& name)
{
	mName = name;
}

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

inline void ITransformable::setTransform(const Transform& transform)
{
	mTransform = transform;
	cache();
}

inline ITransformable::Transform ITransformable::transform() const
{
	return mTransform;
}

inline ITransformable::Transform ITransformable::invTransform() const
{
	return mInvTransformCache;
}

inline float ITransformable::volumeScalefactor() const
{
	return mJacobianDeterminant;
}

inline Eigen::Matrix3f ITransformable::normalMatrix() const
{
	return mNormalMatrixCache;
}

inline Eigen::Matrix3f ITransformable::invNormalMatrix() const
{
	return mInvNormalMatrixCache;
}
} // namespace PR
