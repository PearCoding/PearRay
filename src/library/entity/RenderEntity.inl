#pragma once

namespace PR
{
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
		if(mFrozen)
			return mWorldBoundingBox_Cache;
		else
			return calcWorldBoundingBox();
	}
}