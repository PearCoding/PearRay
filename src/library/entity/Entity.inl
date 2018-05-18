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

inline void Entity::setTransform(const Transform& transform)
{
	mTransform = transform;
}

inline const Entity::Transform& Entity::transform() const
{
	return mTransform;
}

inline const Entity::Transform& Entity::invTransform() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mInvTransformCache;
}

inline const Eigen::Matrix3f& Entity::directionMatrix() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mNormalMatrixCache;
}

inline const Eigen::Matrix3f& Entity::invDirectionMatrix() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mInvNormalMatrixCache;
}
}
