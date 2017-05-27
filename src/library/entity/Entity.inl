#pragma once

namespace PR {
inline uint32 Entity::id() const
{
	return mID;
}

inline void Entity::setName(const std::string& name)
{
	mName = name;
}

inline std::string Entity::name() const
{
	return mName;
}

inline std::string Entity::type() const
{
	return "null";
}

inline bool Entity::isRenderable() const
{
	return false;
}

inline void Entity::setFlags(uint8 f)
{
	mFlags = f;
}

inline uint8 Entity::flags() const
{
	return mFlags;
}

inline void Entity::setPosition(const Eigen::Vector3f& pos)
{
	mPosition = pos;
	invalidateCache();
}

inline Eigen::Vector3f Entity::position() const
{
	return mPosition;
}

inline void Entity::setScale(const Eigen::Vector3f& scale)
{
	mScale = scale;
	invalidateCache();
}

inline Eigen::Vector3f Entity::scale() const
{
	return mScale;
}

inline void Entity::setRotation(const Eigen::Quaternionf& quat)
{
	mRotation = quat;
	invalidateCache();
}

inline Eigen::Quaternionf Entity::rotation() const
{
	return mRotation;
}

inline const Eigen::Affine3f& Entity::transform() const
{
	if (mReCache) {
		mReCache = false;

		mTransformCache		  = Eigen::Translation3f(mPosition) * mRotation * Eigen::Scaling(mScale);
		mInvTransformCache	= mTransformCache.inverse();
		mNormalMatrixCache	= mTransformCache.linear().inverse().transpose();
		mInvNormalMatrixCache = mTransformCache.linear().transpose();
	}

	return mTransformCache;
}

inline const Eigen::Affine3f& Entity::invTransform() const
{
	if (mReCache)
		transform();

	return mInvTransformCache;
}

inline const Eigen::Matrix3f& Entity::directionMatrix() const
{
	if (mReCache)
		transform();
	return mNormalMatrixCache;
}

inline const Eigen::Matrix3f& Entity::invDirectionMatrix() const
{
	if (mReCache)
		transform();
	return mInvNormalMatrixCache;
}

inline void Entity::invalidateCache()
{
	mReCache = true;
	mFrozen  = false;
}

inline void Entity::freeze()
{
	if (!mFrozen) {
		onFreeze();
		mFrozen = true;
	}
}

inline bool Entity::isFrozen() const
{
	return mFrozen;
}
}
