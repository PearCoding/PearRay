// IWYU pragma: private, include "entity/IEntity.h"
namespace PR {
inline bool IEntity::isRenderable() const { return true; }
inline bool IEntity::isCollidable() const { return false; }
inline float IEntity::collisionCost() const { return 1; }

inline BoundingBox IEntity::worldBoundingBox() const
{
	if (mWorldBoundingBox_Cache.isValid())
		return mWorldBoundingBox_Cache;
	else
		return calcWorldBoundingBox();
}

inline size_t IEntity::containerID() const { return mContainerID; }
inline void IEntity::setContainerID(size_t id) { mContainerID = id; }
} // namespace PR
