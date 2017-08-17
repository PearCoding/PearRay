#pragma once

namespace PR {
inline bool RenderEntity::isRenderable() const
{
	return true;
}

inline bool RenderEntity::isCollidable() const
{
	return false;
}

inline float RenderEntity::collisionCost() const
{
	return 1;
}

inline BoundingBox RenderEntity::worldBoundingBox() const
{
	if (isFrozen())
		return mWorldBoundingBox_Cache;
	else
		return calcWorldBoundingBox();
}

inline uint32 RenderEntity::containerID() const
{
	return mContainerID;
}

inline void RenderEntity::setContainerID(uint32 id)
{
	mContainerID = id;
}
}
