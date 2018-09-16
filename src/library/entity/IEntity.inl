#pragma once

namespace PR {
inline bool IEntity::isRenderable() const
{
	return true;
}

inline bool IEntity::isCollidable() const
{
	return false;
}

inline float IEntity::collisionCost() const
{
	return 1;
}

inline BoundingBox IEntity::worldBoundingBox() const
{
	if (isFrozen())
		return mWorldBoundingBox_Cache;
	else
		return calcWorldBoundingBox();
}

inline uint32 IEntity::containerID() const
{
	return mContainerID;
}

inline void IEntity::setContainerID(uint32 id)
{
	mContainerID = id;
}
} // namespace PR
