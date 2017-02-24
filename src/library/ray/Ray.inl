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
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		calcMaxDirectionElement();
#endif
	}

	inline PM::vec3 Ray::direction() const
	{
		return mDirection;
	}

	inline void Ray::setPixelX(uint32 p)
	{
		mPixelX = p;
	}

	inline uint32 Ray::pixelX() const
	{
		return mPixelX;
	}

	inline void Ray::setPixelY(uint32 p)
	{
		mPixelY = p;
	}

	inline uint32 Ray::pixelY() const
	{
		return mPixelY;
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

	inline uint8 Ray::wavelength() const
	{
		return mWavelengthIndex;
	}

	inline void Ray::setWavelength(uint8 wavelength)
	{
		PR_ASSERT(wavelength < Spectrum::SAMPLING_COUNT,
			"Given wavelenght greater than SAMPLING_COUNT");
		mWavelengthIndex = wavelength;
	}

	inline void Ray::setFlags(uint16 flags)
	{
		mFlags = flags;
	}

	inline uint16 Ray::flags() const
	{
		return mFlags;
	}

	inline Ray Ray::next(const PM::vec3& pos, const PM::vec3& dir) const
	{
		return safe(mPixelX, mPixelY, pos, dir, mDepth + 1, mTime, mWavelengthIndex, mFlags);
	}

	inline Ray Ray::safe(uint32 px, uint32 py, const PM::vec3& pos, const PM::vec3& dir, uint32 depth,
		float time, uint8 wavelength, uint16 flags)
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

		return Ray(px, py, posOff, dir, depth, time, wavelength, flags);
	}

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
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
#endif
}
