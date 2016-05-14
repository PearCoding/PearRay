#pragma once

namespace PR
{
	inline void Ray::setStartPosition(const PM::vec3& p)
	{
		mStartPosition = p;
	}

	inline PM::vec3 Ray::startPosition() const
	{
		return mStartPosition;
	}

	inline void Ray::setDirection(const PM::vec3& dir)
	{
		mDirection = dir;
	}

	inline PM::vec3 Ray::direction() const
	{
		return mDirection;
	}

	inline void Ray::setDepth(uint32 depth)
	{
		mDepth = depth;
	}

	inline uint32 Ray::depth() const
	{
		return mDepth;
	}

	inline uint32 Ray::maxDepth() const
	{
		return mMaxDepth;
	}

	inline void Ray::setMaxDepth(uint32 i)
	{
		mMaxDepth = i;
	}

	inline int Ray::flags() const
	{
		return mFlags;
	}

	inline void Ray::setFlags(int flags)
	{
		mFlags = flags;
	}
}