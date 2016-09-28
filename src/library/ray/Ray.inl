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
		calcMaxDirectionElement();
	}

	inline PM::vec3 Ray::direction() const
	{
		return mDirection;
	}

	inline void Ray::setPixel(const PM::vec2& dir)
	{
		mPixel = dir;
	}

	inline PM::vec2 Ray::pixel() const
	{
		return mPixel;
	}

	inline void Ray::setDepth(uint32 depth)
	{
		mDepth = depth;
	}

	inline uint32 Ray::depth() const
	{
		return mDepth;
	}

	inline float Ray::time() const
	{
		return mTime;
	}

	inline void Ray::setTime(float t)
	{
		mTime = t;
	}

	inline void Ray::setFlags(uint16 flags)
	{
		mFlags = flags;
	}

	inline uint16 Ray::flags() const
	{
		return mFlags;
	}

	inline uint32 Ray::maxDepth() const
	{
		return mMaxDepth;
	}

	inline void Ray::setMaxDepth(uint32 i)
	{
		mMaxDepth = i;
	}

	inline Ray Ray::next(const PM::vec3& pos, const PM::vec3& dir) const
	{
		return safe(mPixel, pos, dir, mDepth + 1, mTime, mFlags, mMaxDepth);
	}

	inline Ray Ray::safe(const PM::vec3& pixel, const PM::vec3& pos, const PM::vec3& dir, uint32 depth, float time, uint16 flags, uint32 maxDepth)
	{
		PM::vec3 off = PM::pm_Scale(dir, RayOffsetEpsilon);
		PM::vec3 posOff = PM::pm_Add(pos, off);

		for (int i = 0; i < 3; ++i)
		{
			if (PM::pm_GetIndex(off, i) > 0)
				posOff = PM::pm_SetIndex(posOff, i,
					std::nextafter(PM::pm_GetIndex(posOff, i), std::numeric_limits<float>::max()));
			else if (PM::pm_GetIndex(off, i) < 0)
				posOff = PM::pm_SetIndex(posOff, i,
					std::nextafter(PM::pm_GetIndex(posOff, i), std::numeric_limits<float>::lowest()));
		}

		return Ray(pixel, posOff, dir, depth, time, maxDepth);
	}

	inline uint32 Ray::maxDirectionIndex() const
	{
		return mMaxDirectionIndex;
	}

	inline void Ray::calcMaxDirectionElement()
	{
		mMaxDirectionIndex = 0;
		float maxVal = 0;
		for (uint32 i = 0; i < 3; ++i)
		{
			const float f = std::abs(PM::pm_GetIndex(mDirection, i));
			if (maxVal < f)
			{
				mMaxDirectionIndex = i;
				maxVal = f;
			}
		}
	}
}