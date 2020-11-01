// IWYU pragma: private, include "entity/IEntity.h"
namespace PR {
inline bool IEntity::isRenderable() const { return true; }
inline bool IEntity::isCollidable() const { return false; }
inline float IEntity::collisionCost() const { return 1; }

inline BoundingBox IEntity::worldBoundingBox() const
{
	auto bb = localBoundingBox().transformed(transform());
	if (bb.isPlanar())
		bb.inflate();
	return bb;
}

inline uint8 IEntity::visibilityFlags() const { return mVisibilityFlags; }
inline void IEntity::setVisibilityFlags(uint8 flags) { mVisibilityFlags = flags; }
} // namespace PR
