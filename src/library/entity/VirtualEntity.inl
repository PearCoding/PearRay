#pragma once

namespace PR {
inline uint32 VirtualEntity::id() const
{
	return mID;
}

inline void VirtualEntity::setName(const std::string& name)
{
	mName = name;
}

inline std::string VirtualEntity::name() const
{
	return mName;
}

inline std::string VirtualEntity::type() const
{
	return "null";
}

inline bool VirtualEntity::isRenderable() const
{
	return false;
}

inline void VirtualEntity::setFlags(uint8 f)
{
	mFlags = f;
}

inline uint8 VirtualEntity::flags() const
{
	return mFlags;
}

inline void VirtualEntity::setTransform(const Transform& transform)
{
	mTransform = transform;
}

inline const VirtualEntity::Transform& VirtualEntity::transform() const
{
	return mTransform;
}

inline const VirtualEntity::Transform& VirtualEntity::invTransform() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mInvTransformCache;
}

inline const Eigen::Matrix3f& VirtualEntity::directionMatrix() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mNormalMatrixCache;
}

inline const Eigen::Matrix3f& VirtualEntity::invDirectionMatrix() const
{
	PR_ASSERT(isFrozen(), "Access outside frozen environment!");
	return mInvNormalMatrixCache;
}
}
